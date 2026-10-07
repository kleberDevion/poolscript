#!/bin/bash
# Roda na VPS os jobs Linux dos workflows do GitHub, cada um num container
# `ubuntu:24.04` novo, como um runner novo.
#
# É o CI enquanto o GitHub recusar os jobs (desde 2026-10-06, pela cobrança da
# conta), e é a conferência de "CI verde" de sempre: verde só vale medido na
# imagem do CI, nunca numa Ubuntu limpa nem na VPS direto.
#
# Uso, na VPS, como root:
#   bash ci_vps.sh <pasta> [job ...]
# com
#   <pasta>/fonte.tar.gz   git archive --format=tar.gz --prefix=fonte/ <commit>
#   <pasta>/passos/        a saída de `jinga scripts/ci_passos.pr <raiz> <pasta>/passos`
#                          (os passos de cada job, tirados do .yml letra por letra)
#   job ...                os jobs a rodar; sem nenhum, os que rodam num push
#                          (padrao=1 no job.txt). O noturno: `robustez`.
# Saída:
#   <pasta>/resumo.txt     "== job <nome>", "passo <n> rc=<rc> :: <nome do passo>",
#                          "== fim <nome> rc=<0|1>" e, no fim de tudo, "== FIM"
#   <pasta>/log/<job>.log  a saída inteira de cada job
#   <pasta>/artefatos/<job>/  o que o job guardaria com upload-artifact
#
# A VPS é dividida com serviços de outros, inclusive um Postgres de produção
# em 127.0.0.1:5432. Por isso: tudo que este roteiro cria leva o prefixo
# psci-<pid>; nenhuma porta é publicada no host; os containers ficam numa rede
# própria; os serviços do job (os bancos do job cobertura) dividem o espaço de
# rede do container do job, como no runner, onde escutam no localhost dele; e
# a limpeza, que roda também em erro e em sinal, só remove o que ESTA execução
# criou (containers, a rede e as imagens que ela baixou).
#
# Diferenças conhecidas do runner do GitHub (nenhuma muda o resultado, pelo
# que o repositório lê):
#   - o node é o do apt da Ubuntu (18), o do runner é o 20; o servidor do
#     editor pede 14 ou mais;
#   - o runner tem centenas de pacotes; aqui entra o que os passos e os
#     testes chamam (sudo, git, make, curl, openssl, python3, time, procps,
#     iproute2, netbase, xz-utils, nodejs, npm) e o libpq do repositório do
#     PostgreSQL, como na imagem do runner;
#   - cada passo tem como prazo o que sobra do timeout-minutes do job.
set -u

[ $# -ge 1 ] || { echo "uso: bash ci_vps.sh <pasta> [job ...]" >&2; exit 64; }
PASTA=$(cd "$1" 2>/dev/null && pwd) || { echo "pasta nao existe: $1" >&2; exit 64; }
shift
[ -f "$PASTA/fonte.tar.gz" ] || { echo "falta $PASTA/fonte.tar.gz" >&2; exit 64; }
[ -d "$PASTA/passos" ] || { echo "falta $PASTA/passos (jinga scripts/ci_passos.pr)" >&2; exit 64; }
command -v docker > /dev/null || { echo "sem docker" >&2; exit 64; }

# Outra execução no ar: não se mexe nela, e duas ao mesmo tempo disputariam a CPU.
JA=$(docker ps -a --format '{{.Names}}' | grep '^psci-' || true)
if [ -n "$JA" ]; then
    echo "ja existe container psci-* de outra execucao (nao apago): $JA" >&2
    exit 65
fi

RUN=psci-$$
RES=$PASTA/resumo.txt
WS=/home/runner/work/poolscript/poolscript
mkdir -p "$PASTA/log" "$PASTA/artefatos"
: > "$RES"

docker images --format '{{.Repository}}:{{.Tag}}' | sort -u > "$PASTA/.imagens_antes"
USADAS="ubuntu:24.04"

limpa() {
    local c im u
    for c in $(docker ps -a --format '{{.Names}}' | grep "^$RUN-" || true); do
        docker rm -f -v "$c" > /dev/null 2>&1
    done
    docker network rm "$RUN" > /dev/null 2>&1
    docker images --format '{{.Repository}}:{{.Tag}}' | sort -u > "$PASTA/.imagens_depois"
    for im in $(comm -13 "$PASTA/.imagens_antes" "$PASTA/.imagens_depois"); do
        for u in $USADAS; do
            if [ "$im" = "$u" ]; then docker rmi "$im" > /dev/null 2>&1; fi
        done
    done
    rm -f "$PASTA/.imagens_antes" "$PASTA/.imagens_depois"
}
fim() { limpa; echo "== FIM" >> "$RES"; }
trap fim EXIT
trap 'exit 130' INT TERM HUP

campo() { grep -m1 "^$1=" "$2" | cut -d= -f2-; }

# A preparação do container: o que a imagem do runner já traz e os passos e
# os testes usam sem instalar. Roda como root, antes do primeiro passo.
PREPARA='
set -eo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
    sudo git make ca-certificates curl python3 openssl time procps iproute2 \
    netbase xz-utils nodejs npm libmysqlclient-dev > /dev/null
# a imagem do runner traz o libpq do repositório do PostgreSQL, não o da Ubuntu
install -d /usr/share/postgresql-common/pgdg
curl -fsSL -o /usr/share/postgresql-common/pgdg/apt.postgresql.org.asc https://www.postgresql.org/media/keys/ACCC4CF8.asc
echo "deb [signed-by=/usr/share/postgresql-common/pgdg/apt.postgresql.org.asc] https://apt.postgresql.org/pub/repos/apt noble-pgdg main" > /etc/apt/sources.list.d/pgdg.list
apt-get update -qq
apt-get install -y -qq --no-install-recommends libpq-dev libpq5 > /dev/null
rm -f /etc/apt/sources.list.d/pgdg.list
apt-get update -qq
useradd -m -s /bin/bash runner
printf "runner ALL=(ALL) NOPASSWD:ALL\nDefaults env_keep += \"DEBIAN_FRONTEND\"\n" > /etc/sudoers.d/runner
chmod 440 /etc/sudoers.d/runner
mkdir -p /home/runner/work/_temp
chown -R runner:runner /home/runner/work
echo "preparado: node $(node --version), libpq $(dpkg-query -W -f="\${Version}" libpq5)"
'

# O equivalente do actions/checkout: o fonte do commit na pasta de trabalho.
CHECKOUT='
set -eo pipefail
rm -rf /home/runner/work/poolscript
mkdir -p /home/runner/work/poolscript
tar xzf /psci/fonte.tar.gz -C /home/runner/work/poolscript
mv /home/runner/work/poolscript/fonte /home/runner/work/poolscript/poolscript
chown -R runner:runner /home/runner/work
'

# Sobe os serviços do job no espaço de rede do container dele. Devolve 1 se
# algum não subir ou não ficar saudável em 2 minutos.
sobe_servicos() {
    local job=$1 c=$2 log=$3 dir=$PASTA/passos/$1 s snome simg sporta senv ssaude hp cp t
    while IFS= read -r s; do
        s=${s#servico=}
        IFS='|' read -r snome simg sporta senv ssaude <<< "$s"
        USADAS="$USADAS $simg"
        hp=${sporta%%:*}; cp=${sporta##*:}
        local se=() extra=() pares=() p
        IFS=',' read -ra pares <<< "$senv"
        for p in "${pares[@]}"; do [ -n "$p" ] && se+=(-e "$p"); done
        # No runner o serviço escuta na porta do container e o job o alcança
        # pela porta mapeada no host. Aqui não há host no meio: o serviço
        # escuta direto na porta que o job usa.
        if [ "$hp" != "$cp" ]; then
            case "$simg" in
                mongo:*) extra=(--port "$hp" --bind_ip_all); ssaude=${ssaude/mongosh /mongosh --port $hp }; ;;
                *) echo "servico $snome: porta $sporta sem receita" >> "$log"; return 1 ;;
            esac
        fi
        echo "## servico $snome ($simg, porta $hp)" >> "$log"
        docker run -d --name "$c-$snome" --network "container:$c" --cpus 1 --memory 1g --memory-swap 1g \
            "${se[@]}" "$simg" "${extra[@]}" >> "$log" 2>&1 || return 1
        t=0
        until docker exec "$c-$snome" sh -c "$ssaude" > /dev/null 2>&1; do
            t=$((t + 1))
            if [ $t -ge 24 ]; then echo "servico $snome nao ficou saudavel em 2 min" >> "$log"; return 1; fi
            sleep 5
        done
        echo "## servico $snome saudavel" >> "$log"
    done < <(grep '^servico=' "$dir/job.txt")
    return 0
}

roda_job() {
    local job=$1 dir=$PASTA/passos/$1
    local c=$RUN-$job log=$PASTA/log/$1.log
    : > "$log"
    echo "== job $job" >> "$RES"
    local tmin; tmin=$(campo timeout "$dir/job.txt"); [ -n "$tmin" ] || tmin=360
    local prazo=$((SECONDS + tmin * 60))
    local envs=() l
    while IFS= read -r l; do envs+=(-e "${l#env=}"); done < <(grep '^env=' "$dir/job.txt")
    envs+=(-e CI=true -e GITHUB_ACTIONS=true -e RUNNER_OS=Linux -e LANG=C.UTF-8
           -e HOME=/home/runner -e USER=runner -e RUNNER_TEMP=/home/runner/work/_temp
           -e GITHUB_WORKSPACE="$WS")

    if ! docker run -d --name "$c" --network "$RUN" --cpus 3 --memory 7g --memory-swap 7g \
            -v "$PASTA/fonte.tar.gz:/psci/fonte.tar.gz:ro" -v "$dir:/psci/passos:ro" \
            ubuntu:24.04 sleep infinity >> "$log" 2>&1; then
        echo "== fim $job rc=1 (o container nao subiu)" >> "$RES"; return
    fi
    echo "## preparacao do container" >> "$log"
    if ! docker exec "$c" bash -c "$PREPARA" >> "$log" 2>&1; then
        echo "== fim $job rc=1 (a preparacao do container falhou)" >> "$RES"
        docker rm -f -v "$c" > /dev/null 2>&1; return
    fi
    if grep -q '^servico=' "$dir/job.txt"; then
        if ! sobe_servicos "$job" "$c" "$log"; then
            echo "== fim $job rc=1 (servico nao subiu)" >> "$RES"
            for l in $(docker ps -a --format '{{.Names}}' | grep "^$c" || true); do docker rm -f -v "$l" > /dev/null 2>&1; done
            return
        fi
    fi

    local falhou=0 info n nome tipo se continua caminho rc resto
    for info in $(ls "$dir"/passo_*.info | sort); do
        n=$(basename "$info" .info); n=${n#passo_}
        nome=$(campo nome "$info"); tipo=$(campo tipo "$info"); se=$(campo se "$info")
        continua=$(campo continua "$info"); caminho=$(campo caminho "$info")
        if [ "$falhou" = 1 ] && [ "$se" != sempre ]; then
            echo "passo $n rc=pulou :: $nome" >> "$RES"; continue
        fi
        resto=$((prazo - SECONDS))
        if [ $resto -le 0 ]; then
            echo "passo $n rc=prazo :: $nome" >> "$RES"; falhou=1; continue
        fi
        echo "## passo $n ($tipo): $nome" >> "$log"
        rc=0
        case "$tipo" in
            checkout)
                docker exec "$c" bash -c "$CHECKOUT" >> "$log" 2>&1 || rc=$? ;;
            run)
                docker exec -u runner -w "$WS" "${envs[@]}" "$c" \
                    nice -n 10 timeout --kill-after=30 "$resto" \
                    bash --noprofile --norc -eo pipefail "/psci/passos/passo_$n.sh" >> "$log" 2>&1 || rc=$? ;;
            artefato)
                # if-no-files-found do upload: ausente não derruba o job
                mkdir -p "$PASTA/artefatos/$job"
                docker cp "$c:$WS/$caminho" "$PASTA/artefatos/$job/" >> "$log" 2>&1 || echo "(sem $caminho)" >> "$log" ;;
            *)
                echo "tipo de passo desconhecido: $tipo" >> "$log"; rc=1 ;;
        esac
        echo "passo $n rc=$rc :: $nome" >> "$RES"
        if [ $rc -ne 0 ] && [ "$continua" != 1 ]; then falhou=1; fi
    done
    echo "== fim $job rc=$falhou" >> "$RES"
    for l in $(docker ps -a --format '{{.Names}}' | grep "^$c" || true); do docker rm -f -v "$l" > /dev/null 2>&1; done
}

# Os jobs: os pedidos, ou os que rodam num push.
JOBS=("$@")
if [ ${#JOBS[@]} -eq 0 ]; then
    for d in "$PASTA"/passos/*/; do
        j=$(basename "$d")
        [ "$(campo padrao "$d/job.txt")" = 1 ] && JOBS+=("$j")
    done
fi
for j in "${JOBS[@]}"; do
    [ -f "$PASTA/passos/$j/job.txt" ] || { echo "job desconhecido: $j" >&2; exit 64; }
done
if [ -f "$PASTA/passos/fora.txt" ]; then
    while IFS= read -r l; do [ -n "$l" ] && echo "fora: $l" >> "$RES"; done < "$PASTA/passos/fora.txt"
fi

docker network create "$RUN" > /dev/null || { echo "a rede $RUN nao foi criada" >&2; exit 1; }
for j in "${JOBS[@]}"; do roda_job "$j"; done
