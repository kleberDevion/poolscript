"""
PoolScript - Interpreter (tree-walking)
Etapa 3 do interpretador. Recebe o AST do parser e executa.

Implementa:
- Erros nomeados do spec: AtributtedValueError, OutputUnexpectedValues, SomeValueUnexpected
- Warning não-fatal: IndexOutOfBoundsWarning (acesso fora de range → Null)
- Null == 0 (igualdade nullish), comparações de magnitude com Null = False
- Imports reais via stdlib (os, json, dotenv, request) + imports de arquivos .ps do usuário
- Método .get_json() em strings e em objetos Response
- Built-ins: post, input
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from pathlib import Path
import re
import sys
from typing import Any

from .lexer import PoolSyntaxError
from .ps_errors import shield as _shield
from .errors import (
    ATTRIBUTTED_VALUE_ERROR,
    OUTPUT_UNEXPECTED_VALUES,
    SOME_VALUE_UNEXPECTED,
    INDEX_OUT_OF_BOUNDS_WARNING,
    warn,
)
from .parser import (
    ActionDecl,
    Assignment,
    BinaryOp,
    Block,
    Call,
    CallArg,
    BreakStmt,
    CatchClause,
    ContinueStmt,
    ModelDecl,
    ModelField,
    CountEachStmt,
    CountEachExpr,
    CountExpr,
    DecoratorStmt,
    DictLiteral,
    ExpressionStmt,
    ForEachStmt,
    IfStmt,
    ImportStmt,
    IndexAccess,
    InterpolatedString,
    LambdaExpr,
    ListLiteral,
    Literal,
    MemberAccess,
    Name,
    Node,
    PoolParseError,
    PostfixOp,
    Program,
    ReturnStmt,
    RunSelfWithStmt,
    SliceAccess,
    TryCatchStmt,
    TupleLiteral,
    TypeName,
    UnaryOp,
    UsingStmt,
    VarDecl,
    WhileStmt,
    EntityDecl,
    BaseCall,
    MemberAssignment,
    MatchStmt,
    MatchCase,
    MatchPattern,
    YieldStmt,
    RaiseStmt,
    ColorStrExpr,
    AwaitExpr,
    EntityField,
    UnpackAssignment,
    parse_source,
)
from .stdlib import resolve_module, resolve_name

# ─── Utilitário de cores ANSI para ColorStrExpr ───────────────────────────────
def _ansi_color(hex_or_name: str) -> str:
    """Converte hex (3 ou 6 dígitos) ou nome em escape ANSI 24-bit."""
    from .lexer import NAMED_COLORS
    val = NAMED_COLORS.get(hex_or_name.lower(), hex_or_name)
    val = val.lstrip("#")
    if len(val) == 3:
        val = "".join(c * 2 for c in val)
    if len(val) != 6:
        return ""
    r = int(val[0:2], 16)
    g = int(val[2:4], 16)
    b = int(val[4:6], 16)
    return f"[38;2;{r};{g};{b}m"

_ANSI_RESET = "[0m"

from .builtins import GLOBAL_BUILTINS
from .stdlib.strmethod_lib import PoolStr
from .stdlib.parsing_lib import Parsing, TransientValue


# PoolRuntimeError now inherits from ps_errors for unified error handling
from .ps_errors import _BasePoolRuntimeError as _PSBaseRuntimeError

class PoolRuntimeError(_PSBaseRuntimeError):
    @staticmethod
    def _unwrap(value) -> "Any":
        """Pop automático — extrai o valor real de um TransientValue e descarta o wrapper."""
        if isinstance(value, TransientValue):
            return value.value
        return value

    def __init__(self, msg: str, node: Node, source: str = "", code: str = "RuntimeError",
                 filename: str = "", call_stack: "list | None" = None):
        # Inicializa diretamente sem chamar super().__init__ com args incompatíveis
        self.msg        = msg
        self.node       = node
        self.source     = source
        self.code       = code
        self.filename   = filename
        self.call_stack = call_stack or []
        # Chama Exception.__init__ direto para não passar pela assinatura da base
        Exception.__init__(self, self.format())

    def _fmt_frame(self, filename: str, line: int, col: int, source: str) -> str:
        """Formata um frame do traceback — estilo PoolScript sem expor internos Python."""
        from .ps_errors import _clean_filename, _USE_COLOR
        src_lines = source.splitlines() if source else []
        src     = src_lines[line - 1] if 0 < line <= len(src_lines) else ""
        pointer = " " * max(0, col - 1) + "^^^"
        fname   = _clean_filename(filename or "<script>")
        cyan    = "\033[36m" if _USE_COLOR else ""
        yellow  = "\033[33m" if _USE_COLOR else ""
        dim     = "\033[2m"  if _USE_COLOR else ""
        reset   = "\033[0m"  if _USE_COLOR else ""
        return (
            f"  {dim}em{reset} {cyan}{fname}{reset}, linha {line}\n"
            f"  | {src}\n"
            f"  | {yellow}{pointer}{reset}"
        )

    def format(self) -> str:
        from .ps_errors import _USE_COLOR, _clean_filename
        bold  = "\033[1m"  if _USE_COLOR else ""
        red   = "\033[31m" if _USE_COLOR else ""
        reset = "\033[0m"  if _USE_COLOR else ""
        parts = [f"{bold}{red}{self.code}{reset}: {self.msg}"]

        # call_stack: lista de frames externos (quem chamou quem)
        # Ordem: do mais externo (entry point) para o mais interno (erro)
        # Imprime do mais externo para o mais interno — igual ao Python
        if self.call_stack:
            bold2 = "\033[1m" if _USE_COLOR else ""
            parts.append(f"\n{bold2}Traceback (arquivo mais recente por último):{reset}")
            # Remove frames duplicados mantendo ordem
            seen = set()
            ordered = []
            # Inverte: call_stack foi construído com append() (mais recente no fim)
            # O frame do erro real está em self.filename/self.node
            for frame in self.call_stack:
                key = (frame[0], frame[1], frame[2])
                if key not in seen:
                    seen.add(key)
                    ordered.append(frame)
            # Imprime do mais externo (início da lista) para o mais interno
            for fname, line, col, fsource in ordered:
                parts.append(self._fmt_frame(fname, line, col, fsource))

        # Frame final: onde o erro realmente ocorreu
        err_fname = _clean_filename(self.filename or "<script>")
        err_line  = getattr(self.node, "line", 0)
        err_col   = getattr(self.node, "col",  0)
        parts.append(self._fmt_frame(self.filename or "<script>", err_line, err_col, self.source))
        return "\n".join(parts)

    def pool_message(self) -> str:
        return self.format()


class PoolModel:
    """Representa um `model` da PoolScript. Valida dicts contra os campos definidos."""

    def __init__(self, name: str, fields: list):
        self.name = name
        self.fields = fields  # lista de ModelField

    def validate(self, data: Any) -> tuple[bool, str]:
        """Retorna (válido, mensagem_de_erro)."""
        if not isinstance(data, dict):
            return False, f"esperado um objeto JSON, recebeu {type(data).__name__}"
        for field in self.fields:
            value = data.get(field.name)
            if value is None:
                return False, f"campo '{field.name}' é obrigatório"
            # Valida tipo
            if field.type_name == "str" and not isinstance(value, str):
                return False, f"campo '{field.name}' deve ser texto"
            if field.type_name == "int" and not isinstance(value, (int, float)):
                return False, f"campo '{field.name}' deve ser número inteiro"
            if field.type_name == "flo" and not isinstance(value, (int, float)):
                return False, f"campo '{field.name}' deve ser número"
            if field.type_name == "bool" and not isinstance(value, bool):
                return False, f"campo '{field.name}' deve ser verdadeiro ou falso"
            # Valida length
            if field.length is not None:
                if field.type_name == "str" and len(str(value)) > field.length:
                    return False, f"campo '{field.name}' excede {field.length} caracteres"
                if field.type_name == "int" and len(str(abs(int(value)))) > field.length:
                    return False, f"campo '{field.name}' excede {field.length} dígitos"
        return True, ""

    def __repr__(self):
        return f"<model {self.name}>"


class ReturnSignal(Exception):
    def __init__(self, value: Any):
        self.value = value


class ContinueSignal(Exception):
    """Sinal do `continue` — ignora e segue o fluxo."""
    pass




class YieldSignal(Exception):
    """Lançada pelo yield."""
    def __init__(self, value):
        self.value = value


class BreakSignal(Exception):
    """Sinal do `break` — para o loop."""
    pass



class PoolGenerator:
    """Generator da PoolScript — executa a action pausando a cada yield."""

    def __init__(self, func, args, kwargs, interpreter):
        self._func        = func
        self._args        = args
        self._kwargs      = kwargs
        self._interpreter = interpreter
        self._exhausted   = False

    def __iter__(self) -> "Any":
        fn     = self._func
        interp = self._interpreter
        args   = list(self._args)

        local_scope = Scope(fn.closure)
        params = fn.params

        if len(args) < len(params):
            for p in params[len(args):]:
                if p in fn.defaults:
                    args.append(interp.eval_expr(fn.defaults[p], fn.closure))

        for name, value in zip(params, args):
            local_scope.define(name, value)

        yield from interp._exec_generator_block(fn.block, local_scope)

    def __repr__(self):
        return f"<generator {self._func.name}>"


class PoolTypeRef(str, Enum):
    STR = "str"
    INT = "int"
    FLO = "flo"
    BOOL = "bool"
    LIST = "list"
    JSON = "json"
    TYPE = "type"


class PoolFuture:
    """
    Resultado de uma chamada `async action`/`async reaction`.

    Executa a função em uma thread separada (ThreadPoolExecutor) e
    encapsula o `concurrent.futures.Future` do Python.

    - `await futuro` bloqueia até o resultado e retorna o valor (ou
      relança a exceção, convertida em PoolRuntimeError).
    - `futuro.done()` — True se já terminou.
    - `futuro.result()` — mesmo que await, chamável diretamente.
    """
    __slots__ = ('_future', '_node', '_source', '_filename')

    def __init__(self, future, node=None, source: str = "", filename: str = ""):
        self._future  = future
        self._node    = node
        self._source  = source
        self._filename = filename

    def done(self) -> bool:
        return self._future.done()

    def result(self, timeout: "float | None" = None) -> Any:
        try:
            return self._future.result(timeout=timeout)
        except _PSBaseRuntimeError:
            raise
        except Exception as e:
            raise PoolRuntimeError(
                f"erro em action async: {e}", self._node, self._source,
                code=type(e).__name__, filename=self._filename,
            ) from e

    def __repr__(self) -> str:
        if self._future.done():
            try:
                result = self._future.result(timeout=0)
                return repr(result) if result is not None else "null"
            except Exception:
                return "null"
        return "(aguardando resultado — use 'await' para obter o valor)"


class Scope:
    __slots__ = ('parent', 'values')

    def __init__(self, parent: "Scope | None" = None) -> None:
        self.parent = parent
        self.values: dict[str, Any] = {}

    def define(self, name: str, value: Any) -> None:
        self.values[sys.intern(name)] = value

    def has_local(self, name: str) -> bool:
        return name in self.values

    def set(self, name: str, value: Any) -> bool:
        # Interna uma única vez aqui — a cadeia de parents é percorrida de
        # forma iterativa (não recursiva) reusando o mesmo `name` já interno,
        # em vez de re-internar e empilhar uma chamada por nível de escopo.
        name = sys.intern(name)
        scope: "Scope | None" = self
        while scope is not None:
            if name in scope.values:
                scope.values[name] = value
                return True
            scope = scope.parent
        return False

    def get(self, name: str) -> Any:
        name = sys.intern(name)
        scope: "Scope | None" = self
        while scope is not None:
            if name in scope.values:
                return scope.values[name]
            scope = scope.parent
        raise KeyError(name)


@dataclass
class UserFunction:
    __slots__ = ('name', 'params', 'block', 'closure', 'defaults', '_nonnull', '_static',
                 '_dataentity_init', 'return_type', 'is_async', 'filename', 'source',
                 '_has_yield_cache')

    def __init__(self, name, params, block, closure, defaults=None, filename="", source=""):
        self.name            = sys.intern(name) if isinstance(name, str) else name
        self.params          = [sys.intern(p) if isinstance(p, str) else p for p in (params or [])]
        self._nonnull        = False
        self._static         = False
        self._dataentity_init= False
        self.return_type     = None
        self.is_async        = False
        self.block    = block
        self.closure  = closure
        self.defaults = defaults or {}
        # Onde a função foi DEFINIDA — usado para tracebacks corretos
        self.filename = filename
        self.source   = source
        # Cache de _has_yield: o AST do corpo é imutável após o parse, então
        # o resultado (é generator ou não) nunca muda entre chamadas — evita
        # re-percorrer a árvore inteira a cada invocação (hot path, inclusive
        # em recursão).
        self._has_yield_cache = None


class Module:
    """Wrapper para módulos importados — permite acesso por ponto: os.getenv."""
    def __init__(self, name: str, exports: dict):
        self._name = name
        for k, v in exports.items():
            setattr(self, k, v)

    def __repr__(self):
        return f"<Module {self._name}>"


class StringWithJson(str):
    """Subclasse de str que adiciona .get_json() / .get() — usado para retornos de funções."""
    def get_json(self, key: str | None = None):
        import json as _json
        try:
            data = _json.loads(self)
        except (ValueError, TypeError):
            return None
        if key is None:
            return data
        if isinstance(data, dict):
            return data.get(key)
        return None

    def get(self, key: str):  # type: ignore[override]
        return self.get_json(key)



class PoolEntityInstance:
    """Instância de uma Entity em PoolScript. Equivalente a um objeto Python."""

    def __init__(self, entity: "PoolEntityClass"):
        # __dict__ de instância — armazena todos os self.x
        object.__setattr__(self, "_ps_entity", entity)
        object.__setattr__(self, "_ps_attrs", {})

    def __getattr__(self, name: str):
        attrs = object.__getattribute__(self, "_ps_attrs")
        if name in attrs:
            return attrs[name]
        entity = object.__getattribute__(self, "_ps_entity")
        # procura método na entity ou na cadeia de herança
        method = entity.find_method(name)
        if method is not None:
            return BoundMethod(instance=self, func=method)
        raise AttributeError(f"atributo '{name}' não encontrado na entity '{entity.name}'")

    def __setattr__(self, name: str, value):
        object.__getattribute__(self, "_ps_attrs")[name] = value

    def __repr__(self):
        entity = object.__getattribute__(self, "_ps_entity")
        attrs = object.__getattribute__(self, "_ps_attrs")
        return f"<{entity.name} {attrs}>"


class BoundMethod:
    """Método ligado a uma instância — injeta self automaticamente."""
    def __init__(self, instance: PoolEntityInstance, func: UserFunction):
        self.instance = instance
        self.func = func
        self._nonnull = getattr(func, "_nonnull", False)

    def __repr__(self):
        return f"<bound action {self.func.name}>"


class StaticMethod:
    """Método estático — chamado direto na Entity sem instância."""
    def __init__(self, func: UserFunction):
        self.func     = func
        self._nonnull = getattr(func, "_nonnull", False)
        self._static  = True

    def __repr__(self):
        return f"<static action {self.func.name}>"


class PoolEntityClass:
    """A própria Entity (a 'classe') — callable para instanciar."""

    def __init__(self, name: str, parents: "list[PoolEntityClass]", methods: dict[str, UserFunction]):
        self.name    = name
        self.parents = parents   # lista de pais (MRO simples: esquerda pra direita)
        self.methods = methods

    # legado — primeiro pai (compatibilidade com código antigo)
    @property
    def parent(self):
        return self.parents[0] if self.parents else None

    def find_method(self, name: str) -> "UserFunction | None":
        if name in self.methods:
            return self.methods[name]
        # MRO: busca nos pais da esquerda pra direita
        for parent in self.parents:
            found = parent.find_method(name)
            if found is not None:
                return found
        return None

    def find_parent(self, name: str) -> "PoolEntityClass | None":
        """Localiza um pai pelo nome — para base(NomePai, ...)."""
        for parent in self.parents:
            if parent.name == name:
                return parent
            found = parent.find_parent(name)
            if found is not None:
                return found
        return None

    def __getattr__(self, name: str):
        """Permite acesso direto: Validacao.CPFvalidacao() — método estático."""
        method = self.find_method(name)
        if method is not None and getattr(method, "_static", False):
            return StaticMethod(func=method)
        raise AttributeError(f"Entity '{self.name}' não tem método estático '{name}'")

    def __repr__(self):
        return f"<Entity {self.name}>"


class Interpreter:
    def __init__(self, source: str = "", filename: str = "<stdin>", is_import: bool = False,
                 import_root: "Path | None" = None):
        self.source = source
        self.filename = filename
        self.output: list[str] = []
        self.globals = Scope()
        # Quando True, blocos run_selfwith_ são ignorados (arquivo sendo importado).
        self.is_import: bool = is_import
        # Raiz do projeto para resolução de imports `from a.b.c import x`.
        # Calculada UMA VEZ no interpretador top-level (a partir do arquivo
        # de entrada) e propagada para todos os sub-interpretadores criados
        # via import — igual ao Python, onde imports são sempre relativos
        # à raiz do projeto, não ao arquivo que faz o import.
        if import_root is not None:
            self._import_root: Path = import_root
        elif filename and filename not in ("<stdin>", "<repl>"):
            self._import_root = Path(filename).parent
        else:
            self._import_root = Path.cwd()
        # Profundidade de chamadas de `action` em curso. Usado pelo
        # `count each` para decidir se um `return` interno deve propagar
        # (dentro de action) ou ser tratado como saída local (top-level).
        self._action_depth: int = 0
        # Executor compartilhado para 'async action'/'async reaction'.
        # Criado sob demanda (lazy) — só se o script usar async de fato.
        self._executor = None
        self._install_builtins()

    def _install_builtins(self) -> None:
        self._install_post()
        self.globals.define("input", self._builtin_input)
        self.globals.define("__name__", self.filename or "__main__")
        self.globals.define("getAll", self._builtin_getall)
        self.globals.define("load", self._builtin_load)
        self.globals.define("map", self._builtin_map)
        self.globals.define("filter", self._builtin_filter)
        for name, fn in GLOBAL_BUILTINS.items():
            self.globals.define(name, fn)
        # ── Manipulação de lista ─────────────────────────────────────────────
        self.globals.define("addEnd",     self._builtin_add_end)
        self.globals.define("removeEnd",  self._builtin_remove_end)
        self.globals.define("addStart",   self._builtin_add_start)
        self.globals.define("removeStart",self._builtin_remove_start)
        # ── async/await ──────────────────────────────────────────────────────
        self.globals.define("sleep",      self._builtin_sleep)
        self.globals.define("gather",     self._builtin_gather)

    def _builtin_add_end(self, lista: Any, valor: Any) -> None:
        """addEnd(lista, valor) — adiciona valor ao final da lista (in-place)."""
        if not isinstance(lista, list):
            raise TypeError(f"addEnd() espera uma lista, recebeu {type(lista).__name__}")
        lista.append(valor)

    def _builtin_remove_end(self, lista: Any) -> Any:
        """removeEnd(lista) — remove e retorna o último item da lista."""
        if not isinstance(lista, list):
            raise TypeError(f"removeEnd() espera uma lista, recebeu {type(lista).__name__}")
        if not lista:
            return None
        return lista.pop()

    def _builtin_add_start(self, lista: Any, valor: Any) -> None:
        """addStart(lista, valor) — adiciona valor no início da lista (in-place)."""
        if not isinstance(lista, list):
            raise TypeError(f"addStart() espera uma lista, recebeu {type(lista).__name__}")
        lista.insert(0, valor)

    def _builtin_remove_start(self, lista: Any) -> Any:
        """removeStart(lista) — remove e retorna o primeiro item da lista."""
        if not isinstance(lista, list):
            raise TypeError(f"removeStart() espera uma lista, recebeu {type(lista).__name__}")
        if not lista:
            return None
        return lista.pop(0)


    def _builtin_sleep(self, seconds) -> None:
        """sleep(segundos) — pausa a execução. Útil dentro de 'async action'
        para simular trabalho demorado sem bloquear outras tasks."""
        import time as _time
        _time.sleep(float(seconds))

    def _builtin_gather(self, *futures) -> list:
        """gather(f1, f2, f3) — aguarda múltiplos PoolFuture e retorna lista
        de resultados na mesma ordem. Equivalente a `await [f1, f2, f3]`."""
        result = []
        for f in futures:
            if isinstance(f, PoolFuture):
                result.append(f.result())
            elif isinstance(f, list):
                result.append([v.result() if isinstance(v, PoolFuture) else v for v in f])
            else:
                result.append(f)
        return result

    def _builtin_getall(self) -> Any:
        from .stdlib.db_lib import getAll
        return getAll()

    def _builtin_load(self, path: str | None = None) -> Any:
        from .stdlib.dotenv_lib import load
        return load(path)

    def _builtin_map(self, lista: Any, func: Any) -> list:
        """map(lista, action(x) { return x * 2 }) → nova lista com func aplicada."""
        if not isinstance(lista, (list, tuple)):
            raise TypeError("map() espera uma lista como primeiro argumento")
        result = []
        for item in lista:
            if isinstance(func, UserFunction):
                local = Scope(func.closure)
                if func.params:
                    local.define(func.params[0], item)
                self._action_depth += 1
                try:
                    self.exec_block(func.block, local, create_child=False)
                    result.append(None)
                except ReturnSignal as sig:
                    result.append(sig.value)
                finally:
                    self._action_depth -= 1
            elif callable(func):
                result.append(func(item))
        return result

    def _builtin_filter(self, lista: Any, func: Any) -> list:
        """filter(lista, action(x) { return x > 2 }) → lista com itens que passaram."""
        if not isinstance(lista, (list, tuple)):
            raise TypeError("filter() espera uma lista como primeiro argumento")
        result = []
        for item in lista:
            if isinstance(func, UserFunction):
                local = Scope(func.closure)
                if func.params:
                    local.define(func.params[0], item)
                self._action_depth += 1
                keep = False
                try:
                    self.exec_block(func.block, local, create_child=False)
                except ReturnSignal as sig:
                    keep = self.truthy(sig.value)
                finally:
                    self._action_depth -= 1
                if keep:
                    result.append(item)
            elif callable(func):
                if func(item):
                    result.append(item)
        return result

    def _install_post(self) -> None:
        import time as _time

        interp = self

        class PostObject:
            """post() com suporte a post.flush() efeito digitação."""

            def __call__(self, *args, **kwargs):
                # post() sem argumentos é no-op. Valores None/Null são
                # impressos como "null" (igual ao print() do Python com um
                # valor None) — inclusive quando vêm de índice fora do limite,
                # de um builtin sem resultado, ou de um `return Null` explícito.
                if not args:
                    return None
                text = " ".join(interp.stringify(arg) for arg in args)
                interp.output.append(text)
                print(text)
                return None

            def flush(self, text: str = "", delay: float = 0.05):
                """Efeito de digitação — escreve caractere por caractere."""
                delay = float(delay)
                if delay > 10.1:
                    delay = 10.1
                if delay < 0:
                    delay = 0
                text = interp.stringify(text)
                for char in text:
                    print(char, end="", flush=True)
                    _time.sleep(delay)
                print()  # quebra de linha no final
                interp.output.append(text)
                return None

            def __repr__(self):
                return "<builtin post>"

        self.globals.define("post", PostObject())

    def _builtin_post(self, *args: Any, **kwargs: Any) -> None:
        text = " ".join(self.stringify(arg) for arg in args)
        self.output.append(text)
        print(text)
        return None

    def _builtin_input(self, prompt: str = "") -> Any:
        """input() sempre retorna str, igual ao Python.
        Para converter, declare o tipo: int x = input() / flo x = input()."""
        return PoolStr(input(str(prompt)))

    def run(self, program: Program) -> list[str]:
        try:
            self.exec_program(program, self.globals)
        except ReturnSignal:
            # `return` no top-level (fora de qualquer action) é silencioso.
            # Antes vazava como erro do CLI quando vinha de um `count each`.
            pass
        return self.output

    def exec_program(self, program: Program, scope: Scope) -> None:
        for stmt in program.statements:
            self.exec_statement(stmt, scope)

    def exec_block(self, block: Block, scope: Scope, create_child: bool = True) -> None:
        target_scope = Scope(scope) if create_child else scope
        for stmt in block.statements:
            self.exec_statement(stmt, target_scope)

    def exec_statement(self, node: Node, scope: Scope) -> Any:
        try:
            if node.__class__ is VarDecl:
                value = self.eval_expr(node.value, scope)
                value = self._coerce_declared_value(node.declared_type, value, node)
                self._check_declared_type(node, value)
                # Spec: OutputUnexpectedValues — redeclaração no mesmo escopo
                if scope.has_local(node.name):
                    raise PoolRuntimeError(
                        f"variável '{node.name}' já declarada neste escopo",
                        node, self.source, code=OUTPUT_UNEXPECTED_VALUES,
                    )
                scope.define(node.name, value)
                return None
            if node.__class__ is Assignment:
                value = self.eval_expr(node.value, scope)
                if node.operator == "=":
                    new_value = value
                    if not scope.set(node.target, new_value):
                        scope.define(node.target, new_value)
                    return None
                else:
                    current = self._safe_get(scope, node.target, node)
                    base = current
                    try:
                        match node.operator:
                            case "+=":
                                new_value = self._safe_add(base, value, node)
                            case "-=":
                                new_value = base - value
                            case "*=":
                                new_value = base * value
                            case "/=":
                                new_value = base / value
                            case "%=":
                                new_value = base % value
                            case _:
                                raise PoolRuntimeError(
                                    f"operador de atribuição não suportado: {node.operator}",
                                    node, self.source,
                                )
                    except _PSBaseRuntimeError:
                        raise
                    except TypeError as exc:
                        raise PoolRuntimeError(
                            str(exc), node, self.source, code=SOME_VALUE_UNEXPECTED,
                        ) from exc
                if not scope.set(node.target, new_value):
                    # variável não declarada ainda — define agora
                    scope.define(node.target, new_value)
                return None
            if node.__class__ is UnpackAssignment:
                value = self.eval_expr(node.value, scope)
                self._destructure(node.targets, value, scope, node)
                return None
            if node.__class__ is ExpressionStmt:
                return self.eval_expr(node.expression, scope)
            if node.__class__ is IfStmt:
                for branch in node.branches:
                    if branch.condition is None or self.truthy(self.eval_expr(branch.condition, scope)):
                        # Não captura ContinueSignal/BreakSignal aqui — devem
                        # propagar para o while/for each que envolve o if,
                        # exatamente como return/raise já propagam.
                        self.exec_block(branch.block, scope)
                        break
                return None
            if node.__class__ is WhileStmt:
                while self.truthy(self.eval_expr(node.condition, scope)):
                    try:
                        self.exec_block(node.block, scope)
                    except ContinueSignal:
                        pass
                    except BreakSignal:
                        break
                return None
            if node.__class__ is ForEachStmt:
                iterable = self.eval_expr(node.iterable, scope)
                # PoolGenerator é iterável
                if isinstance(iterable, PoolGenerator):
                    iterable = list(iterable)
                if not isinstance(iterable, (list, tuple, str)):
                    raise PoolRuntimeError("for each exige lista, tupla ou string", node.iterable, self.source)
                for item in iterable:
                    loop_scope = Scope(scope)
                    loop_scope.define(node.item_name, item)
                    try:
                        self.exec_block(node.block, loop_scope, create_child=False)
                    except ContinueSignal:
                        pass
                    except BreakSignal:
                        break
                return None
            if node.__class__ is ActionDecl:
                _fn = UserFunction(name=node.name, params=node.params, block=node.block, closure=scope,
                                   defaults=node.defaults or {}, filename=self.filename, source=self.source)
                _fn.return_type = getattr(node, 'return_type', None)
                _fn.is_async    = getattr(node, 'is_async', False)
                scope.define(node.name, _fn)
                return None
            if node.__class__ is ReturnStmt:
                value = None if node.value is None else self.eval_expr(node.value, scope)
                raise ReturnSignal(value)
            if node.__class__ is RaiseStmt:
                value = self.eval_expr(node.value, scope)
                if isinstance(value, Exception):
                    raise value
                raise PoolRuntimeError(str(value), node, self.source, filename=self.filename)
            if node.__class__ is YieldStmt:
                value = None if node.value is None else self.eval_expr(node.value, scope)
                raise YieldSignal(value)
            if node.__class__ is ContinueStmt:
                raise ContinueSignal()
            if node.__class__ is BreakStmt:
                raise BreakSignal()
            if node.__class__ is TryCatchStmt:
                _error_to_reraise = None
                try:
                    self.exec_block(node.try_block, scope)
                except ReturnSignal:
                    raise
                except BreakSignal:
                    raise
                except ContinueSignal:
                    raise
                except Exception as exc:
                    message = exc.msg if isinstance(exc, _PSBaseRuntimeError) else str(exc)
                    if isinstance(exc, _PSBaseRuntimeError) and exc.code:
                        exc_type = exc.code
                    else:
                        exc_type = type(exc).__name__

                    matched = False
                    for clause in node.catches:
                        if clause.error_type is None or clause.error_type == exc_type:
                            catch_scope = Scope(scope)
                            catch_scope.define(clause.error_name, message)
                            self.exec_block(clause.block, catch_scope, create_child=False)
                            matched = True
                            break

                    if not matched:
                        _error_to_reraise = exc
                finally:
                    # finally sempre executa — com ou sem erro
                    if node.finally_block is not None:
                        self.exec_block(node.finally_block, scope)
                if _error_to_reraise is not None:
                    raise _error_to_reraise
                return None
            if node.__class__ is ImportStmt:
                self._exec_import(node, scope)
                return None
            if node.__class__ is DecoratorStmt:
                dec = node.decorator

                # ── @NonNull — valida que nenhum argumento é Null ────────
                if dec.path == ["static"]:
                    # @static fora de Entity — registra a action normalmente
                    # e marca como _static (pode ser importada de qualquer lugar)
                    if node.block is not None:
                        keys_before = set(scope.values.keys())
                        self.exec_block(node.block, scope, create_child=False)
                        keys_after = set(scope.values.keys())
                        for fn_name in (keys_after - keys_before):
                            fn_val = scope.values[fn_name]
                            if isinstance(fn_val, UserFunction):
                                fn_val._static = True
                    return None

                if dec.path == ["NonNull"]:
                    if node.block is None:
                        raise PoolRuntimeError(
                            "@NonNull deve ser seguido de uma action",
                            node, self.source, filename=self.filename,
                        )
                    # snapshot das keys antes — só a nova action deve ser marcada
                    keys_before = set(scope.values.keys())
                    self.exec_block(node.block, scope, create_child=False)
                    keys_after = set(scope.values.keys())
                    new_keys = keys_after - keys_before
                    # marca SOMENTE a action recém-registrada
                    for fn_name in new_keys:
                        fn_val = scope.values[fn_name]
                        if isinstance(fn_val, UserFunction):
                            fn_val._nonnull = True
                    return None

                if len(dec.path) >= 2:
                    obj = self._safe_get(scope, dec.path[0], node)
                    method_name = dec.path[1]
                    # @app.middleware() — registra middleware
                    if method_name == "middleware" and not dec.args:
                        from .stdlib.jinker_lib import MiddlewareRegistrar
                        registrar = obj._middleware_decorator() if hasattr(obj, "_middleware_decorator") else None
                    # @app.socket(...) — registra websocket
                    elif method_name == "socket" and hasattr(obj, "socket"):
                        args, kwargs = self._eval_call_args(dec.args, scope)
                        registrar = obj.socket(*args, **kwargs)
                    elif hasattr(obj, method_name):
                        args, kwargs = self._eval_call_args(dec.args, scope)
                        registrar = getattr(obj, method_name)(*args, **kwargs)
                    else:
                        registrar = None
                else:
                    registrar = None

                if node.block is not None:
                    block_scope = Scope(scope)
                    self.exec_block(node.block, block_scope, create_child=False)
                    if registrar is not None and hasattr(registrar, "register"):
                        handlers = [
                            v for v in block_scope.values.values()
                            if isinstance(v, UserFunction)
                        ]
                        if handlers:
                            action_fn = handlers[0]
                            interp = self

                            # Socket handler — injeta request e channel no escopo
                            from .stdlib.jinker_lib import _SocketRegistrar
                            if isinstance(registrar, _SocketRegistrar):
                                def make_socket_handler(fn: UserFunction, app_obj):
                                    def socket_handler():
                                        from .stdlib.jinker_lib import request as req_proxy
                                        local = Scope(fn.closure)
                                        local.define("request", req_proxy)
                                        if hasattr(app_obj, "channel"):
                                            local.define("channel", app_obj.channel)
                                        interp._action_depth += 1
                                        try:
                                            interp.exec_block(fn.block, local, create_child=False)
                                        except ReturnSignal:
                                            pass
                                        except ContinueSignal:
                                            pass
                                        finally:
                                            interp._action_depth -= 1
                                    return socket_handler

                                registrar.register(make_socket_handler(action_fn, obj))
                            else:
                                def make_handler(fn: UserFunction):
                                    def handler(req, res):
                                        import json as _json
                                        from .stdlib.jinker_lib import JinkerResponse, request as req_proxy
                                        req_proxy._set(req)
                                        local = Scope(fn.closure)
                                        local.define("request", req_proxy)
                                        interp._action_depth += 1
                                        result = None
                                        try:
                                            interp.exec_block(fn.block, local, create_child=False)
                                        except ReturnSignal as sig:
                                            result = sig.value
                                        except ContinueSignal:
                                            result = "CONTINUE"
                                        finally:
                                            interp._action_depth -= 1

                                        if result == "CONTINUE" or result is None:
                                            return None

                                        if isinstance(result, tuple) and len(result) == 2:
                                            body, status = result
                                            if isinstance(body, JinkerResponse):
                                                body.status_code = int(status)
                                                return body
                                            res.status_code = int(status)
                                            res._body = _json.dumps(body, ensure_ascii=False)
                                            res._content_type = "application/json; charset=utf-8"
                                            return res

                                        if isinstance(result, JinkerResponse):
                                            return result

                                        if isinstance(result, (dict, list)):
                                            res.status_code = 200
                                            res._body = _json.dumps(result, ensure_ascii=False)
                                            res._content_type = "application/json; charset=utf-8"
                                            return res

                                        res.status_code = 200
                                        res._body = str(result)
                                        res._content_type = "text/plain; charset=utf-8"
                                        return res

                                    return handler

                                registrar.register(make_handler(action_fn))
                return None
            if node.__class__ is UsingStmt:
                resource = self.eval_expr(node.resource, scope)
                child = Scope(scope)
                child.define(node.var_name, resource)
                try:
                    if hasattr(resource, "__enter__"):
                        with resource as bound:
                            child.define(node.var_name, bound)
                            self.exec_block(node.block, child, create_child=False)
                    else:
                        try:
                            self.exec_block(node.block, child, create_child=False)
                        finally:
                            if hasattr(resource, "close"):
                                try:
                                    resource.close()
                                except Exception:
                                    pass
                except ReturnSignal:
                    raise
                return None
            if node.__class__ is CountEachStmt:
                return self._exec_count_each(node, scope)
            if node.__class__ is ModelDecl:
                model = PoolModel(name=node.name, fields=node.fields)
                scope.define(node.name, model)
                return None

            # ── Entity declaration ────────────────────────────────────────
            if node.__class__ is EntityDecl:
                # resolve lista de pais
                parent_entities: list = []
                for pname in node.parents:
                    try:
                        pe = scope.get(pname)
                    except KeyError:
                        raise PoolRuntimeError(
                            f"Entity pai '{pname}' não encontrada",
                            node, self.source, filename=self.filename,
                        )
                    if not isinstance(pe, PoolEntityClass):
                        raise PoolRuntimeError(
                            f"'{pname}' não é uma Entity",
                            node, self.source, filename=self.filename,
                        )
                    parent_entities.append(pe)
                # registra métodos
                methods: dict = {}
                pending_nonnull = False
                pending_static  = False
                for method_node in node.body:
                    if isinstance(method_node, DecoratorStmt):
                        if method_node.decorator.path == ["NonNull"]:
                            pending_nonnull = True
                        elif method_node.decorator.path == ["static"]:
                            pending_static = True
                        continue
                    if isinstance(method_node, ActionDecl):
                        fn = UserFunction(
                            name=method_node.name,
                            params=method_node.params,
                            block=method_node.block,
                            closure=scope,
                            defaults=method_node.defaults or {},
                            filename=self.filename, source=self.source,
                        )
                        if pending_nonnull:
                            fn._nonnull = True
                            pending_nonnull = False
                        if pending_static:
                            fn._static = True
                            pending_static = False
                        methods[method_node.name] = fn
                # @dataentity — gerar __init__ automático a partir dos campos
                if node.fields and "__init__" not in methods:
                    methods["__init__"] = self._make_dataentity_init(node.fields, scope)

                entity_class = PoolEntityClass(
                    name=node.name,
                    parents=parent_entities,
                    methods=methods,
                )
                # marca como dataentity para as funções de conversão
                entity_class._dataentity = bool(node.fields)
                scope.define(node.name, entity_class)
                return None

            # ── BaseCall (base(...)) — só faz sentido dentro de __init__ ──
            if node.__class__ is BaseCall:
                try:
                    instance = scope.get("self")
                except KeyError:
                    raise PoolRuntimeError(
                        "base() só pode ser chamado dentro de um __init__ de Entity",
                        node, self.source, filename=self.filename,
                    )
                try:
                    owner_entity = scope.get("__owner_entity__")
                except KeyError:
                    owner_entity = object.__getattribute__(instance, "_ps_entity")

                # herança múltipla: base(NomePai, args...) — alvo específico
                if node.target is not None:
                    entity_cls = object.__getattribute__(instance, "_ps_entity")
                    parent = entity_cls.find_parent(node.target)
                    if parent is None:
                        raise PoolRuntimeError(
                            f"Entity pai '{node.target}' não encontrada na cadeia de herança",
                            node, self.source, filename=self.filename,
                        )
                else:
                    # herança simples: usa owner_entity para resolver
                    if not owner_entity.parents:
                        raise PoolRuntimeError(
                            f"Entity '{owner_entity.name}' não tem pai para chamar base()",
                            node, self.source, filename=self.filename,
                        )
                    parent = owner_entity.parents[0]

                parent_init = parent.find_method("__init__")
                if parent_init is not None:
                    args, kwargs = self._eval_call_args(node.args, scope)
                    self._call_method(parent_init, instance, args, kwargs, node, owner_entity=parent)
                return None

            # ── self.x = valor ────────────────────────────────────────────
            if node.__class__ is MemberAssignment:
                target_obj = self.eval_expr(node.target, scope)
                value = self.eval_expr(node.value, scope)
                if isinstance(target_obj, PoolEntityInstance):
                    setattr(target_obj, sys.intern(node.member), value)
                elif hasattr(target_obj, sys.intern(node.member)):
                    setattr(target_obj, sys.intern(node.member), value)
                else:
                    raise PoolRuntimeError(
                        f"não é possível atribuir membro '{sys.intern(node.member)}' em {type(target_obj).__name__}",
                        node, self.source, filename=self.filename,
                    )
                return None

            # ── match/case ────────────────────────────────────────────────
            if node.__class__ is MatchStmt:
                subject_val = self.eval_expr(node.subject, scope)
                for case in node.cases:
                    matched, bindings = self._match_pattern(case.pattern, subject_val, scope)
                    if matched:
                        # verifica guard
                        if case.pattern.guard is not None:
                            case_scope = Scope(scope)
                            for k, v in bindings.items():
                                case_scope.define(k, v)
                            if not self.eval_expr(case.pattern.guard, case_scope):
                                continue
                        else:
                            case_scope = Scope(scope)
                            for k, v in bindings.items():
                                case_scope.define(k, v)
                        try:
                            self.exec_block(case.body, case_scope, create_child=False)
                        except ReturnSignal:
                            raise
                        break
                return None

            if node.__class__ is RunSelfWithStmt:
                if self.is_import:
                    return None  # arquivo sendo importado — run_selfwith_ ignorado
                self.exec_block(node.block, scope)
                return None
            raise PoolRuntimeError(f"statement não suportado: {type(node).__name__}", node, self.source, filename=self.filename)
        except _PSBaseRuntimeError as _pse:
            # Adiciona frame do CHAMADOR apenas se for arquivo/linha diferente do erro
            if node and self.filename:
                err_fname = _pse.filename or ""
                err_line  = getattr(_pse.node, "line", -1) if _pse.node else -1
                # Só adiciona frame se for um nível diferente (arquivo ou linha diferentes)
                is_new = (self.filename != err_fname) or (node.line != err_line)
                if is_new:
                    frame = (self.filename, node.line, node.col, self.source)
                    if frame not in _pse.call_stack:
                        _pse.call_stack.append(frame)
            raise
        except ReturnSignal:
            raise
        except ContinueSignal:
            raise
        except BreakSignal:
            raise
        except Exception as exc:
            raise _shield(exc, node, self.source, self.filename) from None

    # ── Imports ─────────────────────────────────────────────────────────
    def _exec_import(self, node: ImportStmt, scope: Scope) -> None:
        # 1. Tenta resolver via stdlib registry
        module_exports = resolve_module(node.module)
        if module_exports is not None:
            module_name = ".".join(node.module)
            if node.mode == "import":
                bind_name = node.module_alias or node.module[-1]
                scope.define(bind_name, Module(module_name, module_exports))
            elif node.mode == "from":
                for name in node.names:
                    if name not in module_exports:
                        raise PoolRuntimeError(
                            f"módulo '{module_name}' não exporta '{name}'", node, self.source,
                        )
                    bind_name = node.name_aliases.get(name, name)
                    scope.define(bind_name, module_exports[name])
            elif node.mode == "push":
                if node.names:
                    for name in node.names:
                        if name not in module_exports:
                            raise PoolRuntimeError(
                                f"módulo '{module_name}' não exporta '{name}'", node, self.source,
                            )
                        bind_name = node.name_aliases.get(name, name)
                        scope.define(bind_name, module_exports[name])
                else:
                    bind_name = node.module_alias or node.module[-1]
                    scope.define(bind_name, Module(module_name, module_exports))
            return

        # 2. Tenta resolver como arquivo .ps do usuário — SEMPRE relativo à
        # raiz do projeto (self._import_root), igual ao Python: um import
        # `from services.controllSmtp import x` dentro de
        # services/service_ctrl_user.ps resolve para <raiz>/services/controllSmtp.ps,
        # não para services/services/controllSmtp.ps.
        base_dir = self._import_root
        rel_path = Path(*node.module).with_suffix(".ps")
        candidate = base_dir / rel_path
        if candidate.is_file():
            sub_source = candidate.read_text(encoding="utf-8")
            sub_program = parse_source(sub_source, str(candidate))
            sub_interp = Interpreter(source=sub_source, filename=str(candidate), is_import=True,
                                      import_root=self._import_root)
            try:
                sub_interp.run(sub_program)
            except _PSBaseRuntimeError as _sub_err:
                # Garante que o filename do erro aponta para o arquivo importado,
                # não para o executor. Adiciona frame do arquivo que fez o import.
                if not _sub_err.filename:
                    _sub_err.filename = str(candidate)
                    _sub_err.source   = sub_source
                import_frame = (self.filename, node.line, node.col, self.source)
                if import_frame not in _sub_err.call_stack:
                    _sub_err.call_stack.append(import_frame)
                raise
            sub_exports = {k: v for k, v in sub_interp.globals.values.items()
                           if k not in {"post", "input", "open", "len", "range", "type"}}
            module_name = ".".join(node.module)
            if node.mode == "import":
                bind_name = node.module_alias or node.module[-1]
                scope.define(bind_name, Module(module_name, sub_exports))
            else:  # from / push
                if node.names:
                    for name in node.names:
                        if name not in sub_exports:
                            raise PoolRuntimeError(
                                f"módulo '{module_name}' não exporta '{name}'", node, self.source,
                            )
                        bind_name = node.name_aliases.get(name, name)
                        scope.define(bind_name, sub_exports[name])
                else:
                    bind_name = node.module_alias or node.module[-1]
                    scope.define(bind_name, Module(module_name, sub_exports))
            return

        # 3. Tenta resolver como lib instalada globalmente via `psl install
        # <arquivo>.ps -asLib` — vive em ~/.poolscript/libs/, independe do
        # diretório do projeto (mesmo tratamento de arquivo local do passo 2).
        module_name = ".".join(node.module)
        from .pkgmgr import global_lib_path
        global_candidate = global_lib_path(module_name)
        if global_candidate is not None:
            sub_source = global_candidate.read_text(encoding="utf-8")
            sub_program = parse_source(sub_source, str(global_candidate))
            sub_interp = Interpreter(source=sub_source, filename=str(global_candidate), is_import=True,
                                      import_root=global_candidate.parent)
            try:
                sub_interp.run(sub_program)
            except _PSBaseRuntimeError as _sub_err:
                if not _sub_err.filename:
                    _sub_err.filename = str(global_candidate)
                    _sub_err.source   = sub_source
                import_frame = (self.filename, node.line, node.col, self.source)
                if import_frame not in _sub_err.call_stack:
                    _sub_err.call_stack.append(import_frame)
                raise
            sub_exports = {k: v for k, v in sub_interp.globals.values.items()
                           if k not in {"post", "input", "open", "len", "range", "type"}}
            if node.mode == "import":
                bind_name = node.module_alias or node.module[-1]
                scope.define(bind_name, Module(module_name, sub_exports))
            else:  # from / push
                if node.names:
                    for name in node.names:
                        if name not in sub_exports:
                            raise PoolRuntimeError(
                                f"módulo '{module_name}' não exporta '{name}'", node, self.source,
                            )
                        bind_name = node.name_aliases.get(name, name)
                        scope.define(bind_name, sub_exports[name])
                else:
                    bind_name = node.module_alias or node.module[-1]
                    scope.define(bind_name, Module(module_name, sub_exports))
            return

        raise PoolRuntimeError(
            f"módulo desconhecido: {'.'.join(node.module)}", node, self.source,
        )

    # ── Avaliação de expressões ────────────────────────────────────────
    def eval_expr(self, node: Node, scope: Scope) -> Any:
        try:
            if node.__class__ is ColorStrExpr:
                text = str(self.eval_expr(node.expr, scope))
                prefix = _ansi_color(node.color)
                if prefix:
                    return f"{prefix}{text}{_ANSI_RESET}"
                return text

            if node.__class__ is AwaitExpr:
                value = self.eval_expr(node.value, scope)
                # await numa lista — gather-like: aguarda todas e retorna lista de resultados
                if isinstance(value, list):
                    return [
                        v.result() if isinstance(v, PoolFuture) else v
                        for v in value
                    ]
                # await em PoolFuture — bloqueia até terminar
                if isinstance(value, PoolFuture):
                    return value.result()
                # await em valor comum — pass-through (não é erro)
                return value

            if node.__class__ is Literal:
                if node.kind == "FSTRING":
                    return self._eval_fstring(str(node.value), scope, node)
                if node.kind == "STR" and isinstance(node.value, str):
                    return sys.intern(node.value)
                return node.value
            if node.__class__ is TypeName:
                return PoolTypeRef(sys.intern(node.name))
            if node.__class__ is InterpolatedString:
                out = []
                for part in node.parts:
                    if isinstance(part, Literal):
                        out.append(self.stringify(part.value))
                    else:
                        out.append(self.stringify(self.eval_expr(part, scope)))
                return "".join(out)
            if node.__class__ is Name:
                if node.value == "full":
                    return "full"
                if node.value == "mei":
                    return "mei"
                return self._safe_get(scope, sys.intern(node.value), node)
            if node.__class__ is LambdaExpr:
                return UserFunction(name="<lambda>", params=node.params, block=node.block, closure=scope,
                                   filename=self.filename, source=self.source)
            if node.__class__ is ListLiteral:
                return [self.eval_expr(item, scope) for item in node.items]
            if node.__class__ is TupleLiteral:
                return tuple(self.eval_expr(item, scope) for item in node.items)
            if node.__class__ is DictLiteral:
                out = {}
                for entry in node.entries:
                    if isinstance(entry.key, Name):
                        # Em dict literal {chave: valor}, a chave Name é o lexema
                        key = entry.key.value
                    else:
                        key = self.eval_expr(entry.key, scope)
                    out[key] = self.eval_expr(entry.value, scope)
                return out
            if node.__class__ is UnaryOp:
                value = self.eval_expr(node.operand, scope)
                return self._eval_unary(node.operator, value, node)
            if node.__class__ is BinaryOp:
                left = self.eval_expr(node.left, scope)
                if node.operator in {"and", "&&"}:
                    return self.truthy(left) and self.truthy(self.eval_expr(node.right, scope))
                if node.operator in {"or", "||"}:
                    return self.truthy(left) or self.truthy(self.eval_expr(node.right, scope))
                right = self.eval_expr(node.right, scope)
                return self._eval_binary(left, node.operator, right, node)
            if node.__class__ is Call:
                fn = self.eval_expr(node.callee, scope)
                args, kwargs = self._eval_call_args(node.args, scope)
                return self._call(fn, args, kwargs, node)
            if node.__class__ is MemberAccess:
                target = self.eval_expr(node.target, scope)
                # str → PoolStr para expor métodos estendidos e encadeamento
                if isinstance(target, str) and not isinstance(target, PoolStr):
                    if sys.intern(node.member) in {"get_json", "get"}:
                        wrapped = StringWithJson(target)
                        return getattr(wrapped, sys.intern(node.member))
                    target = PoolStr(target)
                # .type() — retorna o tipo PoolScript do valor
                if node.member == "type":
                    def _type_of(v):
                        from .stdlib.parsing_lib import TransientValue
                        if isinstance(v, TransientValue):
                            return v.origin_type
                        if isinstance(v, bool):    return "bool"
                        if isinstance(v, int):     return "int"
                        if isinstance(v, float):   return "flo"
                        if isinstance(v, str):     return "str"
                        if isinstance(v, list):    return "list"
                        if isinstance(v, dict):    return "dict"
                        if isinstance(v, tuple):   return "tup"
                        if v is None:              return "Null"
                        return type(v).__name__
                    _t = target
                    return lambda: _type_of(_t)

                # acesso estático: Validacao.CPFvalidacao
                if isinstance(target, PoolEntityClass):
                    method = target.find_method(sys.intern(node.member))
                    if method is not None and getattr(method, "_static", False):
                        return StaticMethod(func=method)
                    raise PoolRuntimeError(
                        f"Entity '{target.name}' não tem método estático '{sys.intern(node.member)}' — instancie primeiro",
                        node, self.source, filename=self.filename,
                    )
                if isinstance(target, PoolStr):
                    if hasattr(target, sys.intern(node.member)):
                        return getattr(target, sys.intern(node.member))
                    raise PoolRuntimeError(f"string não tem método '{sys.intern(node.member)}'", node, self.source, filename=self.filename)
                if isinstance(target, dict):
                    _member = sys.intern(node.member)
                    # Métodos nativos de dict — comportamento idêntico ao Python
                    if _member == "get":
                        return target.get
                    if _member == "keys":
                        return lambda: list(target.keys())
                    if _member == "values":
                        return lambda: list(target.values())
                    if _member == "items":
                        return lambda: list(target.items())
                    if _member == "has" or _member == "contains":
                        return lambda k: k in target
                    if _member == "pop":
                        return target.pop
                    if _member == "update":
                        return target.update
                    if _member == "clear":
                        return target.clear
                    if _member == "copy":
                        return lambda: dict(target)
                    if _member == "len":
                        return len(target)
                    # Acesso por chave
                    if _member in target:
                        return target[_member]
                    raise PoolRuntimeError(
                        "chave '" + _member + "' não encontrada no dict"
                        " — use colchetes: ty['" + _member + "'] ou ty.get('" + _member + "')",
                        node, self.source, filename=self.filename
                    )

                # int/flo (NUNCA bool) — métodos de inspeção de string funcionam
                # via conversão automática para string. Importante porque input()
                # auto-converte "150" → int, e o usuário ainda pode querer
                # checar isdigit()/isalpha()/etc no valor original.
                # bool é subclasse de int em Python, mas true.isdigit() não faz
                # sentido semântico — por isso excluído explicitamente.
                if (isinstance(target, (int, float))
                        and not isinstance(target, bool)
                        and not isinstance(target, PoolStr)):
                    _member = sys.intern(node.member)
                    if _member == "len":
                        # len() em número não faz sentido — mantém erro claro
                        pass
                    elif hasattr(PoolStr, _member):
                        as_str = PoolStr(str(target))
                        return getattr(as_str, _member)
                if hasattr(target, sys.intern(node.member)):
                    return getattr(target, sys.intern(node.member))
                raise PoolRuntimeError(f"membro inexistente: {sys.intern(node.member)}", node, self.source, filename=self.filename)
            if node.__class__ is SliceAccess:
                target = self.eval_expr(node.target, scope)
                start = self.eval_expr(node.start, scope) if node.start is not None else None
                stop = self.eval_expr(node.stop, scope) if node.stop is not None else None
                step = self.eval_expr(node.step, scope) if node.step is not None else None
                try:
                    return target[start:stop:step]
                except Exception as e:
                    raise PoolRuntimeError(str(e), node, self.source, filename=self.filename)
            if node.__class__ is IndexAccess:
                target = self.eval_expr(node.target, scope)
                index = self.eval_expr(node.index, scope)
                # Spec: IndexOutOfBoundsWarning — não trava, retorna Null e avisa
                if isinstance(target, (list, tuple, str)) and isinstance(index, int):
                    if index < -len(target) or index >= len(target):
                        warn(
                            f"índice {index} fora do tamanho {len(target)}",
                            node.line, node.col, code=INDEX_OUT_OF_BOUNDS_WARNING,
                        )
                        return None
                return target[index]
            if node.__class__ is PostfixOp:
                if not isinstance(node.operand, Name):
                    raise PoolRuntimeError("++/-- só funcionam em variáveis", node, self.source, filename=self.filename)
                current = self._safe_get(scope, node.operand.value, node)
                updated = current + 1 if node.operator == "++" else current - 1
                if not scope.set(node.operand.value, updated):
                    raise PoolRuntimeError(f"variável não definida: {node.operand.value}", node, self.source, filename=self.filename)
                return current
            if node.__class__ is CountExpr:
                container = self.eval_expr(node.container, scope)
                value = None if node.value_node is None else self.eval_expr(node.value_node, scope)
                return self._count_value(node.target_type, value, container, node)
            if node.__class__ is CountEachExpr:
                container = self.eval_expr(node.container, scope)
                value = None if node.value_node is None else self.eval_expr(node.value_node, scope)
                return self._count_value(node.target_type, value, container, node)
            # BaseCall como expressão (base(...) dentro de __init__)
            if node.__class__ is BaseCall:
                try:
                    instance = scope.get("self")
                except KeyError:
                    raise PoolRuntimeError(
                        "base() só pode ser chamado dentro de um __init__ de Entity",
                        node, self.source, filename=self.filename,
                    )
                try:
                    owner_entity = scope.get("__owner_entity__")
                except KeyError:
                    owner_entity = object.__getattribute__(instance, "_ps_entity")
                if node.target is not None:
                    entity_cls = object.__getattribute__(instance, "_ps_entity")
                    parent = entity_cls.find_parent(node.target)
                    if parent is None:
                        raise PoolRuntimeError(
                            f"Entity pai '{node.target}' não encontrada",
                            node, self.source, filename=self.filename,
                        )
                else:
                    if not owner_entity.parents:
                        raise PoolRuntimeError(
                            f"Entity '{owner_entity.name}' não tem pai para chamar base()",
                            node, self.source, filename=self.filename,
                        )
                    parent = owner_entity.parents[0]
                parent_init = parent.find_method("__init__")
                if parent_init is not None:
                    args, kwargs = self._eval_call_args(node.args, scope)
                    self._call_method(parent_init, instance, args, kwargs, node, owner_entity=parent)
                return None
            raise PoolRuntimeError(f"expressão não suportada: {type(node).__name__}", node, self.source, filename=self.filename)
        except _PSBaseRuntimeError as _pse:
            if node and self.filename and _pse.filename and self.filename != _pse.filename:
                if (self.filename, node.line, node.col, self.source) not in _pse.call_stack:
                    _pse.call_stack.insert(0, (self.filename, node.line, node.col, self.source))
            raise
        except Exception as exc:
            raise _shield(exc, node, self.source, self.filename) from None

    def _eval_call_args(self, args: list[CallArg], scope: Scope) -> tuple[list[Any], dict[str, Any]]:
        positional: list[Any] = []
        named: dict[str, Any] = {}
        for arg in args:
            value = self.eval_expr(arg.value, scope)
            if arg.name is None:
                positional.append(value)
            else:
                named[arg.name] = value
        return positional, named


    def _exec_generator_block(self, block, scope):
        """Executa recursivamente um bloco, fazendo yield dos YieldSignals."""
        for stmt in block.statements:
            yield from self._exec_generator_stmt(stmt, scope)

    def _exec_generator_stmt(self, stmt, scope):
        """Executa um statement dentro de um generator, propagando yields."""
        from poolscript.parser import WhileStmt, ForEachStmt, IfStmt, Block

        if isinstance(stmt, WhileStmt):
            while self.eval_expr(stmt.condition, scope):
                try:
                    yield from self._exec_generator_block(stmt.block, scope)
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue
            return

        if isinstance(stmt, ForEachStmt):
            iterable = self.eval_expr(stmt.iterable, scope)
            if isinstance(iterable, PoolGenerator):
                iterable = list(iterable)
            blk = getattr(stmt, "block", None) or getattr(stmt, "body", None)
            for item in (iterable or []):
                child = Scope(scope)
                child.define(stmt.item_name, item)
                try:
                    if blk:
                        yield from self._exec_generator_block(blk, child)
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue
            return

        if isinstance(stmt, IfStmt):
            for branch in stmt.branches:
                if branch.condition is None or self.eval_expr(branch.condition, scope):
                    blk = getattr(branch, "block", None) or getattr(branch, "body", None)
                    if blk:
                        yield from self._exec_generator_block(blk, scope)
                    break
            return

        # YieldStmt — entrega valor diretamente
        if isinstance(stmt, YieldStmt):
            value = None if stmt.value is None else self.eval_expr(stmt.value, scope)
            yield value
            return

        # statement normal — executa
        try:
            self.exec_statement(stmt, scope)
        except YieldSignal as y:
            yield y.value
        except ReturnSignal:
            return

    def _has_yield(self, block) -> bool:
        """Verifica recursivamente se um bloco contém yield."""
        if not hasattr(block, "statements"):
            return False
        for stmt in block.statements:
            if isinstance(stmt, YieldStmt):
                return True
            # IfStmt — branches com .block ou .body
            if hasattr(stmt, "branches"):
                for b in stmt.branches:
                    blk = getattr(b, "block", None) or getattr(b, "body", None)
                    if blk and self._has_yield(blk):
                        return True
            # ForEachStmt, WhileStmt — .block
            if hasattr(stmt, "block") and stmt.block and self._has_yield(stmt.block):
                return True
            # fallback .body
            if hasattr(stmt, "body") and stmt.body and self._has_yield(stmt.body):
                return True
        return False

    def _exec_generator(self, block, scope):
        """Executa um bloco como generator — yield entrega valores."""
        for stmt in block.statements:
            try:
                result = self.exec_block(
                    type("B", (), {"statements": [stmt], "style": "brace"})(),
                    scope, create_child=False
                )
            except YieldSignal as y:
                yield y.value
            except ReturnSignal:
                return
        # recursivo para blocos aninhados (if, while, for each)

    def _match_pattern(self, pattern: "MatchPattern", value, scope) -> "tuple[bool, dict]":
        """Tenta casar value com pattern. Retorna (matched, bindings)."""
        kind = pattern.kind

        if kind == "wildcard":
            return True, {}

        if kind == "value":
            return value == pattern.value, {}

        if kind == "capture":
            # captura com guard — só registra o binding, guard é checado depois
            return True, {pattern.name: value}

        if kind == "or":
            for pat in pattern.patterns:
                matched, bindings = self._match_pattern(pat, value, scope)
                if matched:
                    return True, bindings
            return False, {}

        if kind == "list":
            if not isinstance(value, list):
                return False, {}
            if len(value) != len(pattern.items):
                return False, {}
            bindings = {}
            for sub_pat, sub_val in zip(pattern.items, value):
                matched, sub_bindings = self._match_pattern(sub_pat, sub_val, scope)
                if not matched:
                    return False, {}
                bindings.update(sub_bindings)
            return True, bindings

        if kind == "dict":
            if not isinstance(value, dict):
                return False, {}
            bindings = {}
            for key, sub_pat in pattern.keys.items():
                if key not in value:
                    return False, {}
                matched, sub_bindings = self._match_pattern(sub_pat, value[key], scope)
                if not matched:
                    return False, {}
                bindings.update(sub_bindings)
            return True, bindings

        return False, {}

    def _exec_function_body(self, fn: "UserFunction", local_scope: "Scope", node: Node) -> Any:
        """Executa o corpo de uma action/reaction e aplica a semântica de
        return_type ('int'/'bool'). Usado para chamadas síncronas e também
        como alvo do ThreadPoolExecutor para 'async action'/'async reaction'.
        """
        self._action_depth += 1
        _rt = getattr(fn, 'return_type', None)
        # Swap filename/source to where the function was DEFINED
        # so tracebacks point to the right file
        _prev_filename = self.filename
        _prev_source   = self.source
        fn_filename = getattr(fn, 'filename', None)
        fn_source   = getattr(fn, 'source',   None)
        if fn_filename:
            self.filename = fn_filename
            self.source   = fn_source or self.source
        try:
            self.exec_block(fn.block, local_scope, create_child=False)
            result = None
        except ReturnSignal as signal:
            result = signal.value
        except _PSBaseRuntimeError:
            if _rt == "int":
                return 500
            elif _rt == "bool":
                return False
            raise
        finally:
            self.filename = _prev_filename
            self.source   = _prev_source
            self._action_depth -= 1
        if _rt == "int":
            return result if isinstance(result, int) else (0 if result is None else result)
        elif _rt == "bool":
            return bool(result) if result is not None else True
        return result

    def _call(self, fn: Any, args: list[Any], kwargs: dict[str, Any], node: Node) -> Any:
        # ── Instanciação de Entity: User("ana", ...) ──────────────────────
        if isinstance(fn, PoolEntityClass):
            instance = PoolEntityInstance(fn)
            init_method = fn.find_method("__init__")
            if init_method is not None:
                self._call_method(init_method, instance, args, kwargs, node)
            return instance

        # ── Chamada de método estático: Validacao.CPFvalidacao() ─────────
        if isinstance(fn, StaticMethod):
            if getattr(fn, "_nonnull", False):
                check_params = [p for p in fn.func.params if p != "self"]
                all_bound = {}
                for i, val in enumerate(args):
                    if i < len(check_params):
                        all_bound[check_params[i]] = val
                all_bound.update(kwargs)
                for p in check_params:
                    if p not in all_bound and p in fn.func.defaults:
                        all_bound[p] = self.eval_expr(fn.func.defaults[p], fn.func.closure)
                for p, val in all_bound.items():
                    if val is None:
                        raise PoolRuntimeError(
                            f"@NonNull: parâmetro '{p}' em '{fn.func.name}' não pode ser Null",
                            node, self.source, filename=self.filename,
                        )
            # chama sem self
            local_scope = Scope(fn.func.closure)
            params = fn.func.params
            if len(args) < len(params):
                filled = list(args)
                for p in params[len(args):]:
                    if p in fn.func.defaults:
                        filled.append(self.eval_expr(fn.func.defaults[p], fn.func.closure))
                    else:
                        raise PoolRuntimeError(
                            f"@static action '{fn.func.name}' faltando argumento: '{p}'",
                            node, self.source, filename=self.filename,
                        )
                args = filled
            for name, value in zip(params, args):
                local_scope.define(name, value)
            self._action_depth += 1
            try:
                self.exec_block(fn.func.block, local_scope, create_child=False)
            except ReturnSignal as signal:
                return signal.value
            finally:
                self._action_depth -= 1
            return None

        # ── Chamada de método ligado: user.falar() ────────────────────────
        if isinstance(fn, BoundMethod):
            if getattr(fn, "_nonnull", False):
                check_params = [p for p in fn.func.params if p != "self"]
                all_bound = {}
                for i, val in enumerate(args):
                    if i < len(check_params):
                        all_bound[check_params[i]] = val
                all_bound.update(kwargs)
                for p in check_params:
                    if p not in all_bound and p in fn.func.defaults:
                        all_bound[p] = self.eval_expr(fn.func.defaults[p], fn.func.closure)
                for p, val in all_bound.items():
                    if val is None:
                        raise PoolRuntimeError(
                            f"@NonNull: parâmetro '{p}' em '{fn.func.name}' não pode ser Null",
                            node, self.source, filename=self.filename,
                        )
            return self._call_method(fn.func, fn.instance, args, kwargs, node)

        if isinstance(fn, UserFunction):
            # ── Detecta generator (contém yield) — cacheado por função ───
            if fn._has_yield_cache is None:
                fn._has_yield_cache = self._has_yield(fn.block)
            if fn._has_yield_cache:
                return PoolGenerator(fn, args, kwargs, self)

            # ── @NonNull — valida antes de executar ──────────────────────
            if getattr(fn, "_nonnull", False):
                # params sem self
                check_params = [p for p in fn.params if p != "self"]
                # resolve args + kwargs para bindings
                all_bound = {}
                for i, val in enumerate(args):
                    if i < len(check_params):
                        all_bound[check_params[i]] = val
                all_bound.update(kwargs)
                # completa com defaults
                for p in check_params:
                    if p not in all_bound and p in fn.defaults:
                        all_bound[p] = self.eval_expr(fn.defaults[p], fn.closure)
                # verifica Null
                for p, val in all_bound.items():
                    if val is None:
                        raise PoolRuntimeError(
                            f"@NonNull: parâmetro '{p}' em '{fn.name}' não pode ser Null",
                            node, self.source, filename=self.filename,
                        )

            local_scope = Scope(fn.closure)
            if kwargs:
                # bind por nome
                bound = {}
                for i, val in enumerate(args):
                    if i < len(fn.params):
                        bound[fn.params[i]] = val
                bound.update(kwargs)
                # aplica defaults para os que faltam
                for p in fn.params:
                    if p not in bound:
                        if p in fn.defaults:
                            bound[p] = self.eval_expr(fn.defaults[p], fn.closure)
                        else:
                            raise PoolRuntimeError(
                                f"action '{fn.name}' faltando argumento: '{p}'",
                                node, self.source, filename=self.filename,
                            )
                for p in fn.params:
                    local_scope.define(p, bound[p])
            else:
                # posicional — completa com defaults
                if len(args) < len(fn.params):
                    filled = list(args)
                    for p in fn.params[len(args):]:
                        if p in fn.defaults:
                            filled.append(self.eval_expr(fn.defaults[p], fn.closure))
                        else:
                            raise PoolRuntimeError(
                                f"action '{fn.name}' faltando argumento: '{p}'",
                                node, self.source, filename=self.filename,
                            )
                    args = filled
                elif len(args) > len(fn.params):
                    raise PoolRuntimeError(
                        f"action '{fn.name}' esperava até {len(fn.params)} argumentos, recebeu {len(args)}",
                        node, self.source,
                    )
                for name, value in zip(fn.params, args):
                    local_scope.define(name, value)
            # 'async action'/'async reaction' — executa em thread separada
            # e retorna um PoolFuture imediatamente, sem bloquear.
            if getattr(fn, 'is_async', False):
                if self._executor is None:
                    from concurrent.futures import ThreadPoolExecutor
                    self._executor = ThreadPoolExecutor(
                        max_workers=8, thread_name_prefix="poolscript-async"
                    )
                _future = self._executor.submit(
                    self._exec_function_body, fn, local_scope, node
                )
                return PoolFuture(_future, node=node, source=self.source, filename=self.filename)

            return self._exec_function_body(fn, local_scope, node)
        if callable(fn):
            return fn(*args, **kwargs)
        raise PoolRuntimeError("tentativa de chamar algo que não é função", node, self.source, filename=self.filename)

    def _make_dataentity_init(self, fields: list, closure: "Scope") -> "UserFunction":
        """Gera um __init__ sintético para @dataentity com base nos campos declarados."""
        from .parser import Block, MemberAssignment, Name, Assignment

        # Constrói bloco de statements: self.campo = campo para cada field
        stmts = []
        params = ["self"]
        defaults = {}

        for field in fields:
            params.append(field.field_name)
            if field.default is not None:
                defaults[field.field_name] = self.eval_expr(field.default, closure)
            else:
                defaults[field.field_name] = None  # sem default = obrigatório (Null sentinel)
            # self.campo = campo  → MemberAssignment(target=Name("self"), member=campo, value=Name(campo))
            stmt = MemberAssignment(
                line=field.line, col=field.col,
                target=Name(line=field.line, col=field.col, value="self"),
                member=field.field_name,
                value=Name(line=field.line, col=field.col, value=field.field_name),
            )
            stmts.append(stmt)

        block = Block(line=0, col=0, style='indent', statements=stmts)
        fn = UserFunction(
            name="__init__",
            params=params,
            block=block,
            closure=closure,
            defaults=defaults,
            filename=self.filename, source=self.source,
        )
        fn._dataentity_init = True
        return fn


    def _call_method(self, func: UserFunction, instance: PoolEntityInstance,
                     args: list[Any], kwargs: dict[str, Any], node: Node,
                     owner_entity: "PoolEntityClass | None" = None) -> Any:
        """Chama um action de Entity injetando self como primeiro parâmetro.
        
        owner_entity: a Entity dona deste método (usada por base() para resolver
        o pai correto na cadeia de herança, evitando loops).
        """
        if not func.params or func.params[0] != "self":
            raise PoolRuntimeError(
                f"action '{func.name}' dentro de Entity deve ter 'self' como primeiro parâmetro",
                node, self.source, filename=self.filename,
            )
        params_without_self = func.params[1:]
        defaults = func.defaults if func.defaults else {}

        if kwargs:
            bound: dict[str, Any] = {}
            for i, val in enumerate(args):
                if i < len(params_without_self):
                    bound[params_without_self[i]] = val
            bound.update(kwargs)
            # aplica defaults para os que faltam
            for p in params_without_self:
                if p not in bound:
                    if p in defaults:
                        bound[p] = self.eval_expr(defaults[p], func.closure)
                    else:
                        raise PoolRuntimeError(
                            f"action '{func.name}' faltando argumento: '{p}'",
                            node, self.source, filename=self.filename,
                        )
            args_final = [bound[p] for p in params_without_self]
        else:
            if len(args) < len(params_without_self):
                filled = list(args)
                for p in params_without_self[len(args):]:
                    if p in defaults:
                        filled.append(self.eval_expr(defaults[p], func.closure))
                    else:
                        raise PoolRuntimeError(
                            f"action '{func.name}' faltando argumento: '{p}'",
                            node, self.source, filename=self.filename,
                        )
                args_final = filled
            elif len(args) > len(params_without_self):
                raise PoolRuntimeError(
                    f"action '{func.name}' esperava até {len(params_without_self)} argumento(s), recebeu {len(args)}",
                    node, self.source, filename=self.filename,
                )
            else:
                args_final = args

        local_scope = Scope(func.closure)
        local_scope.define("self", instance)
        # Injeta __owner_entity__ para que base() resolva o pai correto
        if owner_entity is not None:
            local_scope.define("__owner_entity__", owner_entity)
        for name, value in zip(params_without_self, args_final):
            local_scope.define(name, value)
        self._action_depth += 1
        try:
            self.exec_block(func.block, local_scope, create_child=False)
        except ReturnSignal as signal:
            return signal.value
        finally:
            self._action_depth -= 1
        return None

    def _eval_unary(self, op: str, value: Any, node: Node) -> Any:
        if op == "+":
            return +value
        if op == "-":
            return -value
        if op in {"not", "Not", "!"}:
            return not self.truthy(value)
        raise PoolRuntimeError(f"operador unário inválido: {op}", node, self.source, filename=self.filename)

    # ── Operador `count` ────────────────────────────────────────────────
    _COUNT_TYPE_CHECKS = {
        "str":  lambda v: isinstance(v, str),
        "int":  lambda v: isinstance(v, int) and not isinstance(v, bool),
        "flo":  lambda v: isinstance(v, float),
        "bool": lambda v: isinstance(v, bool),
        "list": lambda v: isinstance(v, list),
        "json": lambda v: isinstance(v, dict),
        # `char`: caractere visível (string len 1, não-branco). Usado para
        # contar caracteres "reais" de uma string ignorando espaços/tabs.
        "char": lambda v: isinstance(v, str) and len(v) == 1 and not v.isspace(),
    }

    def _count_iter_matches(self, target_type: str, value: Any, container: Any, node: Node):
        """Gera (índice/posição, item_capturado) para cada ocorrência válida.

        Comportamento por contêiner:
          - list/tuple: itera elementos; precisa bater no tipo e (se value!=None) ser igual.
          - dict: itera (chave, valor); o valor precisa bater no tipo (e em value, se dado).
          - str: se value é str, conta substring; se value é int, conta o(s) dígito(s)
                 desse número como substring no texto.
          - int (container): converte para str e conta dígitos do `value` (int) lá dentro.
        """
        check = self._COUNT_TYPE_CHECKS.get(target_type)
        if check is None:
            raise PoolRuntimeError(
                f"tipo desconhecido em 'count': {target_type}", node, self.source,
            )
        # Container list/tuple
        if isinstance(container, (list, tuple)):
            for idx, item in enumerate(container):
                if not check(item):
                    continue
                if value is None or self.equals(item, value):
                    yield idx, item
            return
        # Container dict
        if isinstance(container, dict):
            for k, v in container.items():
                if not check(v):
                    continue
                if value is None or self.equals(v, value):
                    yield k, v
            return
        # Container string
        if isinstance(container, str):
            # sem valor → conta caracteres "do tipo" não faz muito sentido.
            if value is None:
                if target_type == "str":
                    # cada caractere é uma str de tamanho 1
                    for idx, ch in enumerate(container):
                        yield idx, ch
                    return
                if target_type == "char":
                    # `char` em string = caracteres visíveis (sem espaços/tabs/quebras)
                    for idx, ch in enumerate(container):
                        if not ch.isspace():
                            yield idx, ch
                    return
                if target_type == "int":
                    for idx, ch in enumerate(container):
                        if ch.isdigit():
                            yield idx, int(ch)
                    return
                return
            needle = str(value) if isinstance(value, (str, int, float, bool)) else None
            if needle is None or needle == "":
                return
            start = 0
            while True:
                pos = container.find(needle, start)
                if pos < 0:
                    return
                yield pos, needle
                start = pos + 1  # permite sobreposições (ex: "aaa" + "aa" → 2)
            return
        # Container int — conta dígitos do valor dentro da representação
        if isinstance(container, int) and not isinstance(container, bool):
            text = str(abs(container))
            if value is None:
                if target_type == "int":
                    for idx, ch in enumerate(text):
                        yield idx, int(ch)
                return
            needle = str(value) if isinstance(value, (int, float, str, bool)) else None
            if needle is None or needle == "":
                return
            start = 0
            while True:
                pos = text.find(needle, start)
                if pos < 0:
                    return
                yield pos, needle
                start = pos + 1
            return
        # Container não suportado → 0 ocorrências
        return

    def _count_value(self, target_type: str, value: Any, container: Any, node: Node) -> int:
        return sum(1 for _ in self._count_iter_matches(target_type, value, container, node))

    def _exec_count_each(self, node: CountEachStmt, scope: Scope) -> Any:
        container = self.eval_expr(node.container, scope)
        value = None if node.value_node is None else self.eval_expr(node.value_node, scope)
        # Pré-conta o total ANTES de iterar, para que `self` (e variáveis
        # auxiliares como `_count`) já valham o total final desde a 1ª iteração.
        # Isso bate com a definição do usuário: "self é o resultado total
        # contado, sempre — não muda durante o loop".
        total = self._count_value(node.target_type, value, container, node)
        in_action = self._action_depth > 0

        for idx, match in self._count_iter_matches(node.target_type, value, container, node):
            child = Scope(scope)
            child.define("self", total)
            child.define("_match", match)
            child.define("_index", idx)
            child.define("_count", total)
            try:
                self.exec_block(node.block, child, create_child=False)
            except ReturnSignal as signal:
                # `return self`, `return <expr>`, `return;`:
                #   - dentro de uma action → propaga para a action devolver o valor.
                #   - no top-level (sem action) → captura e usa como saída local
                #     (não estoura no CLI). O valor da expressão `count each`
                #     é o total contado.
                if in_action:
                    if signal.value is None:
                        raise ReturnSignal(total)
                    raise
                # top-level: encerra o `count each` silenciosamente.
                return total
        return total


    def _safe_add(self, left: Any, right: Any, node: Node):
        # Spec: AtributtedValueError em str + int (e tipos diferentes em geral pra +)
        if isinstance(left, str) and not isinstance(right, str):
            raise PoolRuntimeError(
                f"não é possível somar str com {type(right).__name__}",
                node, self.source, code=ATTRIBUTTED_VALUE_ERROR,
            )
        if isinstance(right, str) and not isinstance(left, str):
            raise PoolRuntimeError(
                f"não é possível somar {type(left).__name__} com str",
                node, self.source, code=ATTRIBUTTED_VALUE_ERROR,
            )
        try:
            return left + right
        except TypeError as exc:
            raise PoolRuntimeError(
                str(exc), node, self.source, code=SOME_VALUE_UNEXPECTED,
            ) from exc

    def _eval_binary(self, left: Any, op: str, right: Any, node: Node) -> Any:
        try:
            if op == "+":
                return self._safe_add(left, right, node)
            if op in {"-", "*", "/", "%"}:
                if isinstance(left, str) or isinstance(right, str):
                    raise PoolRuntimeError(
                        f"operação matemática inválida entre {type(left).__name__} e {type(right).__name__}",
                        node, self.source, code=SOME_VALUE_UNEXPECTED,
                    )
                if op == "-":
                    return left - right
                if op == "*":
                    return left * right
                if op == "/":
                    return left / right
                if op == "%":
                    return left % right
            if op in {"==", "==="}:
                return self.equals(left, right)
            if op in {"!=", "!=="}:
                return not self.equals(left, right)
            # Comparações de magnitude com Null sempre False (spec)
            if op in {"<", ">", "<=", ">="} and (left is None or right is None):
                return False
            if op == "<":
                return left < right
            if op == ">":
                return left > right
            if op == "<=":
                return left <= right
            if op == ">=":
                return left >= right
            if op == "is":
                return self._is_value(left, right)
            if op in {"not is", "is not"}:
                return not self._is_value(left, right)
            if op == "in":
                return left in right
            if op == "not in":
                return left not in right
        except _PSBaseRuntimeError:
            raise
        except TypeError as exc:
            raise PoolRuntimeError(
                str(exc), node, self.source, code=SOME_VALUE_UNEXPECTED,
            ) from exc
        raise PoolRuntimeError(f"operador inválido: {op}", node, self.source, filename=self.filename)

    def _is_value(self, left: Any, right: Any) -> bool:
        if isinstance(right, PoolTypeRef):
            if right == PoolTypeRef.STR:
                return isinstance(left, str)
            if right == PoolTypeRef.INT:
                return isinstance(left, int) and not isinstance(left, bool)
            if right == PoolTypeRef.FLO:
                return isinstance(left, float)
            if right == PoolTypeRef.BOOL:
                return isinstance(left, bool)
            if right == PoolTypeRef.LIST:
                return isinstance(left, list)
            if right == PoolTypeRef.JSON:
                return isinstance(left, dict)
            if right == PoolTypeRef.TYPE:
                return isinstance(left, PoolTypeRef)
        if right is None:
            return left is None
        return left is right or self.equals(left, right)

    def equals(self, left: Any, right: Any) -> bool:
        # data == User → valida o dict contra o model
        if isinstance(right, PoolModel):
            valid, _ = right.validate(left)
            return valid
        if isinstance(left, PoolModel):
            valid, _ = left.validate(right)
            return valid
        # Spec: Null == 0 → True
        if left is None and right == 0:
            return True
        if right is None and left == 0:
            return True
        if left is None and right is None:
            return True
        return left == right

    def truthy(self, value: Any) -> bool:
        if value is None:
            return False
        return bool(value)

    def stringify(self, value: Any) -> str:
        if value is None:
            return "null"
        if isinstance(value, PoolFuture):
            if value._future.done():
                try:
                    result = value._future.result(timeout=0)
                    return self.stringify(result)
                except Exception:
                    return "null"
            return "(aguardando resultado — use 'await' para obter o valor)"
        if value is True:
            return "True"
        if value is False:
            return "False"
        return str(value)

    def _eval_fstring(self, template: str, scope: Scope, node: Node) -> str:
        result = []
        i = 0
        while i < len(template):
            if template[i] == '{' and i + 1 < len(template) and template[i+1] != '{':
                # Encontra o fechamento da chave — suporta chaves aninhadas
                depth = 1
                j = i + 1
                while j < len(template) and depth > 0:
                    if template[j] == '{':
                        depth += 1
                    elif template[j] == '}':
                        depth -= 1
                    j += 1
                expr_str = template[i+1:j-1].strip()
                try:
                    from .parser import parse_source as _ps
                    prog = _ps(expr_str, "<fstring>")
                    if prog.statements:
                        val = self.eval_expr(prog.statements[0].expression
                              if hasattr(prog.statements[0], 'expression')
                              else prog.statements[0], scope)
                    else:
                        val = expr_str
                    result.append(self.stringify(val))
                except Exception:
                    # fallback — tenta como nome simples
                    try:
                        result.append(self.stringify(self._safe_get(scope, expr_str, node)))
                    except Exception:
                        result.append('{' + expr_str + '}')
                i = j
            elif template[i] == '{' and i + 1 < len(template) and template[i+1] == '{':
                result.append('{')
                i += 2
            elif template[i] == '}' and i + 1 < len(template) and template[i+1] == '}':
                result.append('}')
                i += 2
            else:
                result.append(template[i])
                i += 1
        return ''.join(result)

    def _destructure(self, target: "Any", value: Any, scope: Scope, node: Node) -> None:
        """Desempacota `value` de acordo com `target` (um `UnpackTarget`),
        recursivamente. Cada alvo-folha usa a mesma semântica de `Assignment`
        simples: `scope.set(...)` se já existir, senão `scope.define(...)`."""
        if isinstance(value, PoolGenerator):
            value = list(value)
        if not isinstance(value, (list, tuple, str)):
            raise PoolRuntimeError(
                f"não é possível desempacotar valor do tipo {type(value).__name__} "
                f"(esperado lista, tupla ou string)",
                node, self.source, code=SOME_VALUE_UNEXPECTED, filename=self.filename,
            )
        seq = list(value)
        elements = target.elements
        star_index = target.star_index

        if star_index is None:
            if len(seq) != len(elements):
                if len(seq) < len(elements):
                    msg = f"valores insuficientes para desempacotar (esperado {len(elements)}, recebido {len(seq)})"
                else:
                    msg = f"valores demais para desempacotar (esperado {len(elements)}, recebido {len(seq)})"
                raise PoolRuntimeError(
                    msg, node, self.source, code=OUTPUT_UNEXPECTED_VALUES, filename=self.filename,
                )
            pairs = list(zip(elements, seq))
        else:
            before = elements[:star_index]
            star_name = elements[star_index]
            after = elements[star_index + 1:]
            minimum = len(before) + len(after)
            if len(seq) < minimum:
                raise PoolRuntimeError(
                    f"valores insuficientes para desempacotar (esperado pelo menos {minimum}, recebido {len(seq)})",
                    node, self.source, code=OUTPUT_UNEXPECTED_VALUES, filename=self.filename,
                )
            star_value = list(seq[len(before): len(seq) - len(after)])
            pairs = list(zip(before, seq[:len(before)]))
            pairs.append((star_name, star_value))
            pairs.extend(zip(after, seq[len(seq) - len(after):]))

        for leaf, bound_value in pairs:
            if isinstance(leaf, str):
                if not scope.set(leaf, bound_value):
                    scope.define(leaf, bound_value)
            else:
                self._destructure(leaf, bound_value, scope, node)

    def _safe_get(self, scope: Scope, name: str, node: Node) -> Any:
        try:
            return scope.get(name)
        except KeyError as exc:
            raise PoolRuntimeError(f"variável não definida: {name}", node, self.source, filename=self.filename) from exc

    def _coerce_declared_value(self, declared_type: str, value: Any, node: "VarDecl | None" = None) -> Any:
        """Conversão explícita por tipo declarado — só ocorre se o
        desenvolvedor escreveu o tipo: `int x = input()` / `flo x = input()`.

        Sem declaração de tipo, nenhuma conversão acontece — igual Python:
        `x = input()` SEMPRE retorna str, não importa o que foi digitado.
        """
        if declared_type == "int" and isinstance(value, str) and not isinstance(value, bool):
            try:
                return int(value.strip())
            except ValueError:
                _name = node.name if node is not None else "?"
                raise PoolRuntimeError(
                    f"não foi possível converter '{value}' para int "
                    f"(declarado como 'int {_name}')",
                    node, self.source, code="ConversionError", filename=self.filename,
                )

        if declared_type == "flo":
            if isinstance(value, int) and not isinstance(value, bool):
                return float(value)
            if isinstance(value, str):
                try:
                    return float(value.strip())
                except ValueError:
                    _name = node.name if node is not None else "?"
                    raise PoolRuntimeError(
                        f"não foi possível converter '{value}' para flo "
                        f"(declarado como 'flo {_name}')",
                        node, self.source, code="ConversionError", filename=self.filename,
                    )

        return value

    def _check_declared_type(self, node: VarDecl, value: Any) -> None:
        expected = node.declared_type
        if expected == "str" and not isinstance(value, str):
            raise PoolRuntimeError(
                f"variável {node.name} esperava str", node, self.source,
                code=ATTRIBUTTED_VALUE_ERROR,
            )
        if expected == "int" and (not isinstance(value, int) or isinstance(value, bool)):
            raise PoolRuntimeError(
                f"variável {node.name} esperava int", node, self.source,
                code=ATTRIBUTTED_VALUE_ERROR,
            )
        if expected == "flo" and (not isinstance(value, (int, float)) or isinstance(value, bool)):
            raise PoolRuntimeError(
                f"variável {node.name} esperava flo", node, self.source,
                code=ATTRIBUTTED_VALUE_ERROR,
            )
        if expected == "bool" and not isinstance(value, bool):
            raise PoolRuntimeError(
                f"variável {node.name} esperava bool", node, self.source,
                code=ATTRIBUTTED_VALUE_ERROR,
            )


def run_source(source: str, filename: str = "<stdin>") -> list[str]:
    program = parse_source(source, filename)
    interpreter = Interpreter(source=source, filename=filename)
    return interpreter.run(program)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("uso: python -m poolscript.interpreter <arquivo.ps>")
        raise SystemExit(1)

    path = Path(sys.argv[1])
    source = path.read_text(encoding="utf-8")

    try:
        run_source(source, str(path))
    except (PoolSyntaxError, PoolParseError, PoolRuntimeError) as exc:
        print(exc)
        raise SystemExit(2)
