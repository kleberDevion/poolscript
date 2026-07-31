"""Language Server Protocol para PoolScript (.ps).

Fornece diagnostics (erros de lexer/parser em tempo real), completions
com escopo real (símbolos do próprio arquivo + libs importadas — nunca
de outro arquivo aberto sem import) e hover com assinatura + docstring
(bloco `\"\"\" ... \"\"\"` logo no início do corpo de uma action/reaction).
"""
