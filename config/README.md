# Nerd-Config

A parser for a simple configuration file format written in C.

## Example

```json
# A sample config file for a generic text editor
font: "JetBrainsMono Nerd Font"
font.size: 14
zoom: 1.5
lineNumbers: true
```

```c
int
main(void)
{
    Cfg cfg;
    CfgEntry *entries = malloc(64 * sizeof(CfgEntry));
    cfg_init(&cfg, entries, 64);

    char err[MAX_ERR_LEN + 1];
    int res = cfg_load("settings.cfg", &cfg, err);

    if (res != 0) {
        fprintf(stderr, "%s\n", err);
        free(entries);
        return 1;
    }

    // Default value ---------------+
    //                              |
    //                              v
    cfg_get_str(cfg, "font", "Noto Sans Mono");
    cfg_get_int(cfg, "font.size", 12);
    cfg_get_float(cfg, "zoom", 1.0);
    cfg_get_bool(cfg, "lineNumbers", true);

    free(entries);
    return 0;
}
```

## Syntax

-   A config file consists of zero or more lines

-   A line is composed of a key, a colon (`:`), a value, and a newline (`\n`)

-   A key is a sequence of one or more alphabetic characters or dots (`.`)

-   A value can be one of the following types: string, boolean, integer, or float

### Data types:

-   **String**: A string is enclosed within double quotes (`"`) and can contain alphabetic characters, punctuation, digits, and blanks (spaces or tabs)

-   **Boolean**: A boolean value is either `true` or `false`

-   **Integer**: An integer is a sequence of digits, which may be preceded by a hyphen (`-`) to indicate a negative value

-   **Float**: A floating-point number consists of a sequence of digits, possibly with a decimal point (`.`), which may be preceded by a hyphen (`-`) to indicate a negative value

## EBNF Grammar

```
cfg   ::= line*
line  ::= key ':' val '\n'
key   ::= (alpha | '.')+
val   ::= str | bool | int | float

str   ::= '"' (alpha | punct | digit | blank)+ '"'
alpha ::= 'a' ... 'z' | 'A' ... 'Z'
punct ::= '.' | ':' | '~' | '!' | ...
blank ::= ' ' | '\t'

bool  ::= true | false

int   ::= '-'? digit+
float ::= '-'? digit+ '.' digit+
digit ::= '0' ... '9'
```
