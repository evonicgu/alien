<div id="top"></div>

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<h3 align="center">Alien</h3>

  <p align="center">
    Compiler frontend generator
    <br />
    <br />
    <br />
    <a href="https://sunfline.github.io/alien-docs">Project documentation</a>
    ·
    <a href="https://github.com/sunfline/alien/issues">Report Bug</a>
    ·
    <a href="https://github.com/sunfline/alien/issues">Request Feature</a>
  </p>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Usage</a></li>
      </ul>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

Alien is a compiler frontend generator, which makes creating lexers and parsers for your new language an easy task. There are many generators out there; however, I tried to make this one comfortable to work with.

Here are the main features:
* Single configuration file for both lexer and parser
* Default templates are customisable with help of %code directives
* If default templates aren't what you want, you can always create your own
* Unicode support; ASCII-only lexers are also possible

<p align="right">(<a href="#top">back to top</a>)</p>



### Built With

Alien is written in C++ and uses CMake as the build system. [CPM](https://github.com/cpm-cmake/CPM.cmake) is the dependency manager being used.

Dependencies:
* [CxxOpts](https://github.com/jarro2783/cxxopts)
* [Utf8proc](https://github.com/JuliaStrings/utf8proc)
* [Inja](https://github.com/pantor/inja)
* [Nlohmann Json](https://github.com/nlohmann/json)

<p align="right">(<a href="#top">back to top</a>)</p>

Documentation:
* [mdBook](https://github.com/rust-lang/mdBook)



<!-- GETTING STARTED -->
## Getting Started

You can set up the generator following these steps

### Prerequisites

You need to have cmake installed on your system

### Installation

_You shouldn't install any of the dependencies yourself since CPM handles the ones that are not included in the ./deps folder_

1. Clone the repo
   ```sh
   git clone https://github.com/sunfline/alien.git && cd alien
   ```
2. Let cmake do its work
   ```sh
   cmake -DCMAKE_BUILD_TYPE=Release -G <your generator (Ninja, VS, etc.)> -S . -B ./cmake-build-release
   cmake --build ./cmake-build-release --target alien
   ```
3. Create a config file
   <details>
        <summary>Config file</summary>

    ```text
    #generation.noutf8 = true       // ascii-only lexer is generated
    #generation.track_lines = false // for performance reasons
    #token.namespace = "lexer"      // to show parser where custom token classes are
    
    %code {
        struct digit_token : public token<token_type> {
            long long value;
        
            digit_token(long long value)
                : token(token_type::digit),
                  value(value) {}
        };
    }
        
    {
        digit = digit_token,
        plus,
        minus,
        mul,
        div,
        exp,
        paren_open,
        paren_close
    }
        
    %left plus minus
    %left mul div
    %right exp
        
    %%
        
    \+:             {}[plus];
    \-:             {}[minus];
    \*:             {}[mul];
    /:              {}[div];
    \^:             {}[exp];
    \(:             {}[paren_open];
    \):             {}[paren_close];
    [1-9][0-9]*:    {
                        return new digit_token(std::stoll(gettext()));
    };
    \s:             {};
        
    %%
        
    #generation.symbol_type = expr_t
        
    %code-top {
        enum class expr_type {
            DIGIT,
            BINARY
        };
        
        enum class op_type {
            PLUS,
            MINUS,
            MUL,
            DIV,
            EXP
        };
        
        struct expr_t {
            expr_type etype;
        
            expr_t(expr_type etype)
                : etype(etype) {}
        
            virtual ~expr_t() = default;
        };
        
        struct digit_expr : public expr_t {
            long long value;
        
            digit_expr(long long value)
                : expr_t(expr_type::DIGIT),
                  value(value) {}
        };
        
        struct binary_expr : public expr_t {
            expr_t *lhs, *rhs;
            op_type otype;
        
            binary_expr(expr_t* lhs, expr_t* rhs, op_type otype)
                : expr_t(expr_type::BINARY),
                  lhs(lhs),
                  rhs(rhs),
                  otype(otype) {}

            ~binary_expr() override {
                delete lhs;
                delete rhs;
            }
        };
    }
        
    {
        expr = expr_t
    }
        
    %%
        
    expr:
    %digit                          {$$ = new digit_expr($0->value);}
    | expr %plus expr               {$$ = new binary_expr($0, $2, op_type::PLUS);}
    | expr %minus expr              {$$ = new binary_expr($0, $2, op_type::MINUS);}
    | expr %mul expr                {$$ = new binary_expr($0, $2, op_type::MUL);}
    | expr %div expr                {$$ = new binary_expr($0, $2, op_type::DIV);}
    | expr %exp expr                {$$ = new binary_expr($0, $2, op_type::EXP);}
    | %paren_open expr %paren_close {$$ = $1;}
    ;

    %%
    ```
   </details>
4. Generate your lexer and parser
   ```sh
   ./alien <config file> -o <output file>
   ```
5. Use them in your code
   ```c++
   #include <iostream>
   #include <fstream>
   #include "parser.out.h"

   int main() {
       lexer::lexer<std::ifstream> l("../content/f.txt");

       parser::parser p(l);

       auto* tree = p.parse(); // pointer to root of the produced AST
   }
   ```
6. Parse some text

   Produced parser is able to parse simple mathematical expressions such as
   ```text
   5 + 5           -> ((5) + (5))
   18 ^ 3 * 3      -> (((18) ^ (3)) * (3))
   2 + 2 / 2       -> ((2) + ((2) / (2)))
   ```
   Here every parenthesized digit or expression represents a single instance of `expr_t` class generated by one of the actions the parser will execute while building the AST.
   <br /><br />
   In fact our code can parse correctly any expression which uses `+` `-` `/` `*` and `^` as operators.

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

- [x] Separate source and header files
- [x] Lexer documentation
- [ ] Parser documentation
- [ ] Proper error messages
- [ ] GLR Parsing 

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Anton - [evonicgu@gmail.com](mailto:evonicgu@gmail.com)

Project Link: [https://github.com/sunfline/alien](https://github.com/sunfline/alien)

<p align="right">(<a href="#top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/sunfline/alien.svg?style=for-the-badge
[contributors-url]: https://github.com/sunfline/alien/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/sunfline/alien.svg?style=for-the-badge
[forks-url]: https://github.com/sunfline/alien/network/members
[stars-shield]: https://img.shields.io/github/stars/sunfline/alien.svg?style=for-the-badge
[stars-url]: https://github.com/sunfline/alien/stargazers
[issues-shield]: https://img.shields.io/github/issues/sunfline/alien.svg?style=for-the-badge
[issues-url]: https://github.com/sunfline/alien/issues
[license-shield]: https://img.shields.io/github/license/sunfline/alien.svg?style=for-the-badge
[license-url]: https://github.com/sunfline/alien/blob/master/LICENSE.txt