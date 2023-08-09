# Simple-Config

A parser for a simple configuration file format.

## Example

```
# A sample config file for a generic text editor
font: "JetBrainsMono Nerd Font"
font.size: 14
zoom: 1.5
lineNumbers: true
bg.color: rgba(255, 255, 255, 1)
```

```c
int
main(void)
{
    Cfg cfg;
    CfgEntry *entries = malloc(64 * sizeof(CfgEntry));
    cfg_init(&cfg, entries, 64);

    char err[CFG_MAX_ERR_LEN + 1];
    int res = cfg_load("sample.cfg", &cfg, err);

    if (res != 0) {
        fprintf(stderr, "%s\n", err);
        free(entries);
        return 1;
    }

    // Default value         --------------------+
    // (if key is not found)                     |
    //                                           v
    char* font = cfg_get_str(cfg, "font", "Noto Sans Mono");
    int font_size = cfg_get_int(cfg, "font.size", 12);
    float zoom = cfg_get_float(cfg, "zoom", 1.0);
    bool line_num = cfg_get_bool(cfg, "lineNumbers", true);
    CfgColor bg_color = cfg_get_color(cfg, "bg.color",
                                      (CfgColor){
                                          .r = 255,
                                          .g = 255,
                                          .b = 255,
                                          .a = 1,
                                      });

    free(entries);
    return 0;
}
```

## Specification

-   A config file must have the `.cfg` file extension

-   A config file consists of zero or more lines

-   A line is composed of a key, a colon (`:`), a value, and a newline (`\n`)

-   A key is a sequence of one or more alphabetic characters or dots (`.`)

-   A value can be one of the following types: string, boolean, integer, float, or color

-   A comment starts with a `#` and can placed on its own line or at the end of a line containing an entry

## Data types

-   **String**: A string is enclosed within double quotes (`"`) and can contain alphabetic characters, punctuation, digits, and blanks (spaces or tabs). Double quotes are not allowed inside a string.

-   **Boolean**: A boolean is either `true` or `false`

-   **Integer**: An integer is a sequence of digits, which may be preceded by a hyphen (`-`) to indicate a negative value

-   **Float**: A floating-point number consists of a sequence of digits with a decimal point (`.`), which may be preceded by a hyphen (`-`) to indicate a negative value

-   **Color**: A color is defined in the `rgba(R, G, B, A)` format, where `R`, `G`, and `B` are integers between 0 and 255, and `A` is a floating-point number between 0 and 1

## EBNF grammar

```
cfg   ::= line*
line  ::= key ':' val '\n'
key   ::= (alpha | '.')+
val   ::= str | bool | int | float | color

str   ::= '"' (alpha | punct | digit | blank)+ '"'
alpha ::= 'a' ... 'z' | 'A' ... 'Z'
punct ::= '.' | ':' | '~' | '!' | ...
blank ::= ' ' | '\t'

bool  ::= true | false

int   ::= '-'? digit+
float ::= '-'? digit+ '.' digit+
digit ::= '0' ... '9'

color ::= 'rgba(' digit+ ',' digit+ ',' digit+ ',' digit+ ')'
```

## Acknowledgements

A big thank you to [Cozis](https://github.com/cozis) for helping me out and giving me lots of advice throughout the entire project.
