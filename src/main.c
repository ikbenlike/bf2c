#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

char *def =
    "//\n"
    "// Generated using bf2c: https://github.com/ikbenlike/bf2c\n"
    "//\n"
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

struct bfargs {
    char *file;
    char *output;
    char *prefix;
    char *postfix;
};

struct bfargs *parse_arg(int argc, char **argv){
    struct bfargs *args = calloc(1, sizeof(struct bfargs));
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--prefix")){
            if(argv[++i] == NULL){
                fprintf(stderr, "bf2c: --prefix: you need to provide a file\n");
                exit(1);
            }
            args->prefix = argv[i];
        }
        else if(!strcmp(argv[i], "--postfix")){
            if(argv[++i] == NULL){
                fprintf(stderr, "bf2c: --postfix: you need to provide a file\n");
                exit(1);
            }
            args->postfix = argv[i];
        }
        else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")){
            if(argv[++i] == NULL){
                fprintf(stderr, "bf2c: -o: you need to provide a filename\n");
                exit(1);
            }
            args->output = argv[i];
        }
        else {
            args->file = argv[i];
        }
    }
    return args;
}

/*char *readfile(FILE *f){
    ssize_t size = 4096;
    char *buf = calloc(size, sizeof(char));
    char *tmp = NULL;
    ssize_t n = 0;
    ssize_t offset = n;
    while((n = fread(buf + offset, sizeof(char), size - offset, f)) > 0){
        offset += n;
        if(offset >= size){
            size *= 2;
            tmp = realloc(buf, size * sizeof(char));
            if(!tmp){
                fprintf(stderr, "bf2c: %s\n", strerror(errno));
                free(buf);
                exit(1);
            }
            buf = tmp;
        }
    }
    puts(buf);
    return buf;
}*/
char *readfile(FILE *fp){
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    rewind(fp);
    char *buffer = calloc(len + 1, sizeof(char));
    fread(buffer, len, 1, fp);
    return buffer;
}

FILE *prepare_output(char *path, char *out, char *prefix){
    size_t len = strlen(path);
    puts(path);
    if(!out){
        out = calloc(len + 6, sizeof(char));
        strcpy(out, path);
        strcat(out, "out.c");
        puts(out);
    }
    FILE *f = fopen(out, "w");
    if(prefix){
        FILE *pre = fopen(prefix, "r");
        char *contents = readfile(pre);
        fwrite(contents, 1, strlen(contents), f);
        fclose(pre);
        free(contents);
        return f;
    }
    fwrite(def, 1, strlen(def), f);
    return f;
}

void finish_output(FILE *out, char *postfix){
    if(postfix){
        FILE *post = fopen(postfix, "r");
        char *contents = readfile(post);
        fclose(post);
        fwrite(contents, 1, strlen(contents), out);
        free(contents);
        return;
    }
    fwrite("    return 0;\n}\n", 1 , 16, out);
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
    FILE *out = prepare_output(args->file, args->output, args->prefix);
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
                    snprintf(tmp, 255, "array[dptr] -= %zu;\n", seq);
                    fwrite(tmp, 1, strlen(tmp), out);
                    iptr += seq - 1;
                    break;
                }
                fwrite("array[dptr]--;\n", 1, 15, out);
                break;
            case '.':
                write_indent(il, out);
                fwrite("putchar(array[dptr]);\n", 1, 22, out);
                break;
            case ',':
                write_indent(il, out);
                fwrite("array[dptr] = getchar();\n", 1, 25, out);
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

    finish_output(out, args->postfix);
    free(code);
    fclose(out);
    fclose(in);
    free(args);
    
    return 0;
}
