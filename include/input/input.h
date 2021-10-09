#ifndef ALIEN_INPUT_H
#define ALIEN_INPUT_H

#include <string>
#include <iostream>

namespace alien::input {

    class input {
    public:
        virtual char get() = 0;

        virtual char peek() = 0;

        input(const input&) = delete;

        input() = default;
    };

    class string_input : public input {
        std::string str;

        unsigned int pos = 0, end;

    public:
        explicit string_input(const std::string& str) : str(str) {
            end = this->str.size();
        }

        explicit string_input(std::string&& str) : str(str) {
            end = this->str.size();
        }

        char get() override {
            return pos == end ? -2 : str[pos++];
        }

        char peek() override {
            return pos == end ? -2 : str[pos];
        }
    };

    class stream_input : public input {
        std::istream& stream;

        char buffer[4096];
        unsigned int pos, max;

        bool end = false;

    public:
        explicit stream_input(std::istream& stream) : stream(stream) {
            fill();
        }

        char get() override {
            if (end) {
                return -2;
            }

            if (pos == max) {
                fill();
            }

            return buffer[pos++];
        }

        char peek() override {
            if (end) {
                return -2;
            }

            if (pos == max) {
                fill();
            }

            return buffer[pos];
        }

    private:
        void fill() {
            stream.read(buffer, 4096);

            pos = 0;
            max = stream.gcount();

            if (max == 0) {
                end = true;
                buffer[0] = -2;
            }
        }
    };

}

#endif //ALIEN_INPUT_H