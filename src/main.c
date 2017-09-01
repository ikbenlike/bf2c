#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

char *def =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "#include <errno.h>\n"
    "int main(void){\n"
    "    char *array = calloc(30000, sizeof(char));\n"
    "    int dptr = 0;\n"
    "    if(array == NULL){\n"
    "        fprintf(stderr, \"%s: %s\\n\", __FILE__, strerror(errno));\n"
    "        exit(1);\n"
    "    }\n";

char *readfile(FILE *f){
    ssize_t size = 4096;
    char *buf = calloc(size, sizeof(char));
    char *tmp = NULL;
    ssize_t n = 0;
    while((n = fread(buf, sizeof(char), size, f)) > 0){
        if(n > size){
            size *= 1.5;
            tmp = realloc(buf, size * sizeof(char));
            if(!tmp){
                fprintf(stderr, "bf2c: %s\n", strerror(errno));
                free(buf);
                exit(1);
            }
        }
    }
    return buf;
}

FILE *prepare_output(char *path){
    size_t len = strlen(path);
    puts(path);
    char *out = calloc(len + 5, sizeof(char));
    strcpy(out, path);
    strcat(out, "out.c");
    puts(out);
    FILE *f = fopen(out, "w");
    fwrite(def, 1, strlen(def), f);
    return f;
}

struct bfargs {
    char *file;
    bool keep;
};

struct bfargs *parse_arg(int argc, char **argv){
    struct bfargs *args = calloc(1, sizeof(struct bfargs));
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--keep") || !strcmp(argv[i], "-k")){
            args->keep = true;
        }
        else {
            args->file = argv[i];
        }
    }
    return args;
}

void write_indent(size_t n, FILE *out){
    for(size_t i = 0; i < n; i++){
        fwrite("    ", 1, 4, out);
    }
}

size_t detect_sequence(char *code, size_t iptr){
    char c = code[iptr];
    size_t count = 0;

    while (code[iptr + count] == c) {
        count++;
    }
    return count;
}

int main(int argc, char **argv){
    struct bfargs *args = parse_arg(argc, argv);
    if(!args){
        fprintf(stderr, "bf2c: %s\n", strerror(errno));
        return 1;
    }
    FILE *out = prepare_output(args->file);
    FILE *in = fopen(args->file, "r");
    
    char *code = readfile(in);
    size_t max = strlen(code);
    size_t iptr = 0;
    size_t il = 1;

    while(iptr < max){
        size_t seq = 1;
        switch(code[iptr]){
            case '>':
                write_indent(il, out);
                seq = detect_sequence(code, iptr);
                if(seq != 1){
                    char tmp[256] = {0};
                    snprintf(tmp, 255, "dptr += %zu;\n", seq);
                    fwrite(tmp, 1, strlen(tmp), out);
                    iptr += seq - 1;
                    break;
                }
                fwrite("dptr++;\n", 1, 8, out);
                break;
            case '<':
                write_indent(il, out);
                seq = detect_sequence(code, iptr);
                if(seq != 1){
                    char tmp[256] = {0};
                    snprintf(tmp, 255, "dptr -= %zu;\n", seq);
                    fwrite(tmp, 1, strlen(tmp), out);
                    iptr += seq - 1;
                    break;
                }
                fwrite("dptr--;\n", 1, 8, out);
                break;
            case '+':
                write_indent(il, out);
                seq = detect_sequence(code, iptr);
                if(seq != 1){
                    char tmp[256] = {0};
                    snprintf(tmp, 255, "array[dptr] += %zu;\n", seq);
                    fwrite(tmp, 1, strlen(tmp), out);
                    iptr += seq - 1;
                    break;
                }
                fwrite("array[dptr]++;\n", 1, 15, out);
                break;
            case '-':
                write_indent(il, out);
                seq = detect_sequence(code, iptr);
                if(seq != 1){
                    char tmp[256] = {0};
                    snprintf(tmp, 255, "array[dptr] -= %zu\n", seq);
                    fwrite(tmp, 1, strlen(tmp), out);
                    iptr += seq - 1;
                    break;
                }
                fwrite("array[dptr]--;\n", 1, 15, out);
                break;
            case '.':
                write_indent(il, out);
                fwrite("printf(\"%c\", array[dptr]);\n", 1, 27, out);
                break;
            case ',':
                write_indent(il, out);
                fwrite("array[dptr] = getc(stdin);\n", 1, 27, out);
                break;
            case '[':
                write_indent(il, out);
                fwrite("while(array[dptr]){\n", 1, 20, out);
                il++;
                break;
            case ']':
                il--;
                write_indent(il, out);
                fwrite("}\n", 1, 2, out);
                break;
            default:
                break;
        }
        iptr++;
    }

    fwrite("    return 0;\n}\n", 1 , 16, out);
    fclose(out);
    
    return 0;
}
