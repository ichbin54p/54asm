#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE* rlog;

int compile_size;
int disk_size = 1024;
int ram_amount = 256 * 3;
int reg_amount = 4;
int port_count = 8;
int ii;

signed int* reg;
unsigned char opl;
unsigned char opv[2];
unsigned char opi;
unsigned char flags[3] = {1, 0, 0};
unsigned char* ram;
unsigned char* opr1;
unsigned char* opr2; // 1op trust

char* disk_path;
char* port_path;

int est_size(char* path){
    FILE* f = fopen(path, "r");
    char* log_path = malloc(strlen(path) + 10);

    sprintf(log_path, "%s.build.log", path);

    FILE* log = fopen(log_path, "w");

    free(log_path);

    int lines = 0;
    int rlines = 0;
    char* temp = malloc(20);

    if (f){
        while (fgets(temp, 20, f)){
            if (temp[0] == '#' || temp[0] == '\n'){
                //
            } else {
                lines += 1;
            }

            rlines += 1;
        }
    } else {
        lines = -1;
    }

    free(temp);
    fclose(f);
    fclose(log);

    return lines;
}

int ifl(char* line, int index){
    int l = strlen(line);
    int c = (l - index);
    int j = 0;
    char* o = malloc(c + 1);

    for (int i = index; i < l; i++){
        if (line[i] == '\n'){
            break;
        }

        o[j] = line[i];
        j += 1;
    }

    o[j] = '\0';

    int a = atoi(o);

    free(o);

    return a;
}

int compile(char* path){
    char* log_path = malloc(strlen(path) + 10);
    char* build_path = malloc(strlen(path) + 6);
    char* disk_path = malloc(strlen(path) + 6 + 4);
    char* ports_path = malloc(strlen(path) + 6 + 5);

    sprintf(log_path, "%s.build.log", path);
    sprintf(build_path, "%s.build", path);
    sprintf(disk_path, "%s.build.disk", path);
    sprintf(ports_path, "%s.build.ports", path);

    FILE* build = fopen(build_path, "w");
    FILE* log = fopen(log_path, "w");
    FILE* disk = fopen(disk_path, "w");
    FILE* ports = fopen(ports_path, "w");
    FILE* f = fopen(path, "r");

    free(log_path);
    free(build_path);
    free(disk_path);
    free(ports_path);

    printf("compile %s: starting...\n", path);

    compile_size = 0;

    int ii = 0;
    int ps = est_size(path) * 3;
    char offset = 64;
    char* line = malloc(500);

    fprintf(log, "est_size returned %d\n", ps);

    for (int i = 0; i < disk_size; i++){
        fprintf(disk, "%c", 0);
    } for (int i = 0; i < port_count; i++){
        fprintf(ports, "%c", 0);
    }

    fprintf(build, "%c", offset);

    while (fgets(line, 500, f)){
        if (line[0] == '#' || line[0] == '\n'){
            fprintf(log, "comment or newline, ignoring\n");
        } else {
            int np = 0;

            fprintf(log, "instruction %d: %c%c%c\n", ii, line[0], line[1], line[2]);

            if (strncmp(line, "LDI", 3) == 0){
                fprintf(build, "%c", 0 + offset);
                ii += 1;
            } else if (strncmp(line, "RST", 3) == 0){
                fprintf(build, "%c", 1 + offset);
                ii += 1;
            } else if (strncmp(line, "RLD", 3) == 0){
                fprintf(build, "%c", 2 + offset);
                ii += 1;
            } else if (strncmp(line, "PST", 3) == 0){
                fprintf(build, "%c", 3 + offset);
                ii += 1;
            } else if (strncmp(line, "PLD", 3) == 0){
                fprintf(build, "%c", 4 + offset);
                ii += 1;
            } else if (strncmp(line, "STR", 3) == 0){
                fprintf(build, "%c", 5 + offset);
                ii += 1;
            } else if (strncmp(line, "LOD", 3) == 0){
                fprintf(build, "%c", 6 + offset);
                ii += 1;
            } else if (strncmp(line, "DST", 3) == 0){
                fprintf(build, "%c", 7 + offset);
                ii += 1;
            } else if (strncmp(line, "DLD", 3) == 0){
                fprintf(build, "%c", 8 + offset);
                ii += 1;
            } else if (strncmp(line, "BRC", 3) == 0){
                fprintf(build, "%c", 9 + offset);
                ii += 1;
            } else if (strncmp(line, "ADD", 3) == 0){
                fprintf(build, "%c", 10 + offset);
                ii += 1;
            } else if (strncmp(line, "SUB", 3) == 0){
                fprintf(build, "%c", 11 + offset);
                ii += 1;
            } else if (strncmp(line, "INC", 3) == 0){
                fprintf(build, "%c", 12 + offset);
                ii += 1;
                np = 1;
            } else if (strncmp(line, "DEC", 3) == 0){
                fprintf(build, "%c", 13 + offset);
                ii += 1;
                np = 1;
            } else if (strncmp(line, "OPS", 3) == 0){
                fprintf(build, "%c", 14 + offset);
                ii += 1;
            } else if (strncmp(line, "OPL", 3) == 0){
                fprintf(build, "%c", 15 + offset);
                ii += 1;
                np = 1;
            } else if (strncmp(line, "OPR", 3) == 0){
                // fprintf(build, "%c", offset);
                ii += 1;
            } else {
                // fprintf(build, "%c", offset - 1);
                fprintf(log, "- Unkown opcode\n");
                ii += 1;
                np = 1;
            }

            if (!np){
                unsigned char operand = (unsigned char) ifl(line, 4);

                fprintf(build, "%c", operand);
                fprintf(log, "- operand: %d\n", operand);
            }
        }
    }

    fclose(build);
    fclose(log);
    fclose(f);
    fclose(disk);
    fclose(ports);
    free(line);
    
    return 0;
}

void update_flags(){
    flags[1] = 0;
    flags[2] = 0;

    if (reg[0] == 0){
        flags[1] = 1;
    } else if (reg[0] > 255){
        flags[2] = 1;
        reg[0] -= 256;
    } else if (reg[0] < 0){
        flags[2] = 1;
        reg[0] += 256;
    }

    fprintf(rlog, "\nFlags update:\n- None: %d\n- Zero: %d\n- Cout: %d\n", flags[0], flags[1], flags[2]);
}

void update(){
    if (opl){
        opl -= 1;
    }
}

void LDI(){
    if (opl){
        fprintf(rlog, "%d: Loading immediate (%d) into reg0 (%d)\n", ii, opv[0], reg[0]);

        reg[0] = opv[0];
    } else {
        fprintf(rlog, "%d: Loading immediate (%d) into reg0 (%d)\n", ii, opr1[ii], reg[0]);

        reg[0] = opr1[ii];
    }

    update_flags();
}

void RST(){
    if (opl){
        fprintf(rlog, "%d: Loading reg0 (%d) into reg%d (%d)\n", ii, reg[0], opv[0] + 1, reg[opv[0]+1]);

        reg[opv[0]+1] = reg[0];
    } else {
        fprintf(rlog, "%d: Loading reg0 (%d) into reg%d (%d)\n", ii, reg[0], opr1[ii] + 1, reg[opr1[ii]+1]);

        reg[opr1[ii]+1] = reg[0];
    }
}

void RLD(){
    if (opl){
        fprintf(rlog, "%d: Loading reg%d (%d) into reg0 (%d)\n", ii, opv[0]+1, reg[opv[0]+1], reg[0]);

        reg[0] = reg[opv[0]+1];
    } else {
        fprintf(rlog, "%d: Loading reg%d (%d) into reg0 (%d)\n", ii, opr1[ii]+1, reg[opr1[ii]+1], reg[0]);

        reg[0] = reg[opr1[ii]+1];
    }
}

void PST(){
    FILE* port_file = fopen(port_path, "r+");

    if (opl){
        fprintf(rlog, "%d: Loading reg0 (%d) into port%d\n", ii, reg[0], opv[0]);
        fseek(port_file, opv[0], SEEK_SET);
        fprintf(port_file, "%c", reg[0]);
    } else {
        fprintf(rlog, "%d: Loading reg0 (%d) into port%d\n", ii, reg[0], opr1[ii]);
        fseek(port_file, opr1[ii], SEEK_SET);
        fprintf(port_file, "%c", reg[0]);
    }

    fclose(port_file);
}

void PLD(){
    FILE* port_file = fopen(port_path, "r");

    if (opl){
        fprintf(rlog, "%d: Loading port%d into reg0 (%d)\n", ii, opv[0], reg[0]);
        fseek(port_file, opv[0], SEEK_SET);

        reg[0] = fgetc(port_file);
    } else {
        fprintf(rlog, "%d: Loading port%d into reg0 (%d)\n", ii, opr1[ii], reg[0]);
        fseek(port_file, opr1[ii], SEEK_SET);

        reg[0] = fgetc(port_file);
    }

    fclose(port_file);
}

void STR(){
    if (opl){
        fprintf(rlog, "%d: Loading reg0 (%d) into ram address %d (%d)\n", ii, reg[0], (opv[0] * 256) + opv[1], ram[(opv[0]*256)+opv[1]]);

        ram[(opv[0]*256)+opv[1]] = reg[0];
    } else {
        fprintf(rlog, "%d: Loading reg0 (%d) into ram address %d (%d)\n", ii, reg[0], (opr1[ii] * 256) + opr2[ii], ram[(opr1[ii]*256)+opr2[ii]]);

        ram[(opr1[ii]*256)+opr2[ii]] = reg[0];
    }
}

void LOD(){
    if (opl){
        fprintf(rlog, "%d: Loading ram address %d (%d) into reg0 (%d)\n", ii, (opv[0] * 256) + opv[1], ram[(opv[0]*256)+opv[1]], reg[0]);

        reg[0] = ram[(opv[0]*256)+opv[1]];
    } else {
        fprintf(rlog, "%d: Loading ram address %d (%d) into reg0 (%d)\n", ii, (opr1[ii] *  256) + opr2[ii], ram[(opr1[ii]*256)+opr2[ii]], reg[0]);

        reg[0] = ram[(opr1[ii]*256)+opr2[ii]];
    }
}

void DST(){
    FILE* disk_file = fopen(disk_path, "r+");

    if (opl){
        fprintf(rlog, "%d: Loading reg0 (%d) into disk address %d\n", ii, reg[0], (opv[0] * 256) + opv[1]);
        fseek(disk_file, (opv[0] * 256) + opv[1], SEEK_SET);
        fprintf(disk_file, "%c", reg[0]);
    } else {
        fprintf(rlog, "%d: Loading reg0 (%d) into disk address %d\n", ii, reg[0], (opr1[ii] * 256) + opr2[ii]);
        fseek(disk_file, (opr1[ii] * 256) + opr2[ii], SEEK_SET);
        fprintf(disk_file, "%c", reg[0]);
    }

    fclose(disk_file);
}

void DLD(){
    FILE* disk_file = fopen(disk_path, "r");

    if (opl){
        fprintf(rlog, "%d: Loading disk address %d into reg0\n", ii, (opv[0] * 256) + opv[1], reg[0]);
        fseek(disk_file, (opv[0] * 256) + opv[1], SEEK_SET);

        reg[0] = fgetc(disk_file);
    } else {
        fprintf(rlog, "%d: Loading disk address %d into reg0\n", ii, (opr1[ii] * 256) + opr2[ii], reg[0]);
        fseek(disk_file, (opr1[ii] * 256) + opr2[ii], SEEK_SET);

        reg[0] = fgetc(disk_file);
    }
    
    fclose(disk_file);
}

void BRC(){
    if (opl){
        fprintf(rlog, "%d: Attempting to branch to instruction %d, flag %d\n", ii, opv[1], opv[0]);

        if (opv[0] < 3){
            if (flags[opv[0]]){
                fprintf(rlog, "- True: jumping (%d)\n", flags[opv[0]]);

                ii = opv[1] - 1;
            } else {
                fprintf(rlog, "- False: ignore (%d)\n", flags[opv[0]]);
            }
        } else {
            fprintf(rlog, "- False: unkown flag\n");
        }
    } else {
        fprintf(rlog, "%d: Attempting to branch to instruction %d, flag %d\n", ii, opr2[ii], opr1[ii]);

        if (opr1[ii] < 3){
            if (flags[opr1[ii]]){
                fprintf(rlog, "- True: jumping (%d)\n", flags[opr1[ii]]);

                ii = opr2[ii] - 1;
            } else {
                fprintf(rlog, "- False: ignore (%d)\n", flags[opr1[ii]]);
            }
        } else {
            fprintf(rlog, "- False: unkown flag\n");
        }
    }
}

void ADD(){
    if (opl){
        fprintf(rlog, "%d: Adding reg0 (%d) with reg%d (%d) = %d\n", ii,  reg[0], opv[0] + 1, reg[opv[0]+1], reg[0] + reg[opv[0]+1]);

        reg[0] += reg[opv[0]+1];
    } else {
        fprintf(rlog, "%d: Adding reg0 (%d) with reg%d (%d) = %d\n", ii,  reg[0], opr1[ii] + 1, reg[opr1[ii]+1], reg[0] + reg[opv[0]+1]);

        reg[0] += reg[opr1[ii]+1];
    }

    update_flags();
}

void SUB(){
    if (opl){
        fprintf(rlog, "%d: Subtracting reg0 (%d) with reg%d (%d) = %d\n", ii,  reg[0], opv[0] + 1, reg[opv[0]+1], reg[0] - reg[opv[0]+1]);

        reg[0] -= reg[opv[0]+1];
    } else {
        fprintf(rlog, "%d: Subtracting reg0 (%d) with reg%d (%d) = %d\n", ii,  reg[0], opr1[ii] + 1, reg[opr1[ii]+1], reg[0] - reg[opv[0]+1]);

        reg[0] -= reg[opr1[ii]+1];
    }

    update_flags();
}

void INC(){
    fprintf(rlog, "%d: Incrementing reg0 (%d)\n", ii, reg[0]);

    reg[0] += 1;

    update_flags();
}

void DEC(){
    fprintf(rlog, "%d: Decrementing reg0 (%d)\n", ii, reg[0]);

    reg[0] -= 1;

    update_flags();
}

void OPS(){
    fprintf(rlog, "%d: Saving operand %d, value = reg0(%d)\n", ii, opr1[ii], reg[0]);

    if (opr1[ii] < 2){
        opv[opr1[ii]] = reg[0];
    } else {
        fprintf(rlog, "- Out of range\n");
    }
}

void OPL(){
    fprintf(rlog, "%d: Loading operand values %d %d\n", ii, opv[0], opv[1]);

    opl = 2;
}

int execute(char* path){
    printf("execute %s: starting...\nallocating memory\n- ram: %dB\n- reg: %dB\n", path, ram_amount * sizeof(char), (reg_amount + 1) * sizeof(char));

    char* log_path = malloc(strlen(path) + 7);
    
    disk_path = malloc(strlen(path) + 5);
    port_path = malloc(strlen(path) + 6);

    sprintf(log_path, "%s.run.log", path);
    sprintf(disk_path, "%s.disk", path);
    sprintf(port_path, "%s.ports", path);

    printf("execute %s: files:\n- %s\n- %s\n- %s\n", path, log_path, disk_path, port_path);

    FILE* temp = fopen(log_path, "w");
    FILE* build = fopen(path, "r");
    FILE* port_file = fopen(port_path, "w");

    fprintf(temp, "--- 54asm: %s run log ---\n\n", path);
    fclose(temp);

    rlog = fopen(log_path, "a");

    free(log_path);

    char* line = malloc(10000);

    ii = 0;

    char offset;
    int i = 0;
    size_t ps = fread(line, 1, 10000, build);
    void (**ins)() = malloc(ps * sizeof(void (*)()));

    ram = malloc(ram_amount * sizeof(char));
    reg = malloc((reg_amount + 1) * sizeof(short));
    opr1 = malloc(ps * sizeof(char));
    opr2 = malloc(ps * sizeof(char));

    printf("execute %s: reseting memory\n- ram: %dB\n- reg: %dB\n- port: %dB\n", path, ram_amount, reg_amount + 1, port_count);

    for (int a = 0; a < reg_amount + 1; a++){
        reg[a] = 0;
    } for (int a = 0; a < port_count; a++){
        fprintf(port_file, "%c", 0);
    }

    fclose(port_file);
    printf("execute %s: converting program, %d bytes\n", path, ps);

    while (i < ps){
        if (i < 1){
            printf("execute %s: %d, detected OFS %d\n", path, i, line[i]);
            offset = line[i];
            i += 1;
        } else {
            switch (line[i] - offset){
                case 0:
                    printf("execute %s: %d, detected LDI %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = LDI;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;
                    
                    break;
                case 1:
                    printf("execute %s: %d, detected RST %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = RST;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;
                    break;
                case 2:
                    printf("execute %s: %d, detected RLD %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = RLD;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;
                    break;
                case 3:
                    printf("execute %s: %d, detected PST %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = PST;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;
                    break;
                case 4:
                    printf("execute %s: %d, detected PLD %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = PLD;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;
                    break;
                case 5:
                    printf("execute %s: %d, detected STR %d %d --> %d\n", path, i, line[i+1], line[i+2], ii);

                    ins[ii] = STR;
                    opr1[ii] = line[i+1];
                    opr2[ii] = line[i+2];

                    i += 3;
                    ii += 1;
                    break;
                case 6:
                    printf("execute %s: %d, detected LOD %d %d --> %d\n", path, i, line[i+1], line[i+2], ii);

                    ins[ii] = LOD;
                    opr1[ii] = line[i+1];
                    opr2[ii] = line[i+2];

                    i += 3;
                    ii += 1;
                    break;
                case 7:
                    printf("execute %s: %d, detected DST %d %d --> %d\n", path, i, line[i+1], line[i+2], ii);

                    ins[ii] = DST;
                    opr1[ii] = line[i+1];
                    opr2[ii] = line[i+2];

                    i += 3;
                    ii += 1;
                    break;
                case 8:
                    printf("execute %s: %d, detected DLD %d %d --> %d\n", path, i, line[i+1], line[i+2], ii);

                    ins[ii] = DLD;
                    opr1[ii] = line[i+1];
                    opr2[ii] = line[i+2];

                    i += 3;
                    ii += 1;
                    break;
                case 9:
                    printf("execute %s: %d, detected BRC %d %d --> %d\n", path, i, line[i+1], line[i+2], ii);

                    ins[ii] = BRC;
                    opr1[ii] = line[i+1];
                    opr2[ii] = line[i+2];

                    i += 3;
                    ii += 1;

                    break;
                case 10:
                    printf("execute %s: %d, detected ADD %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = ADD;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;

                    break;
                case 11:
                    printf("execute %s: %d, detected SUB %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = SUB;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;

                    break;
                case 12:
                    printf("execute %s: %d, detected INC --> %d\n", path, i, ii);

                    ins[ii] = INC;

                    i += 1;
                    ii += 1;

                    break;
                case 13:
                    printf("execute %s: %d, detected DEC --> %d\n", path, i, ii);

                    ins[ii] = DEC;

                    i += 1;
                    ii += 1;

                    break;
                case 14:
                    printf("execute %s: %d, detected OPS %d --> %d\n", path, i, line[i+1], ii);

                    ins[ii] = OPS;
                    opr1[ii] = line[i+1];

                    i += 2;
                    ii += 1;

                    break;
                case 15:
                    printf("execute %s: %d, detected OPL --> %d\n", path, i, ii);

                    ins[ii] = OPL;

                    i += 1;
                    ii += 1;

                    break;
                default:
                    printf("execute %s: %d, detected UNK %d %d --> %d \n", path, i, offset, line[i], ii);

                    i += 1;
                    break;
            }
        }

        // printf("%d < %d\n", i, ps);
    }

    printf("execute %s: executing program (length: %d) with offset %d\n", path, ii, offset);

    int pl = ii;
    int ei = 0;
    ii = 0;

    while (ii < pl){
        update();

        if (ins[ii] != NULL){
            ins[ii]();
            ii += 1;
        } else {
            printf("error: null pointer at index: %d\n", ii);
            break;
        }

        ei += 1;

        fprintf(rlog, "\n");
        fflush(rlog);
        usleep(100);
    }

    printf("execute %s: executed %d instructions, freeing memory\n", path, ei);

    fclose(build);
    fclose(rlog);
    free(line);
    free(ram);
    free(reg);
    free(ins);
    free(disk_path);
    free(port_path);

    return 0;
}

int main(int argc, char** argv){
    int i = 1;

    while (i < argc){
        if (strcmp(argv[i], "-i") == 0){
            if (compile(argv[i+1]) != 0){
                printf("error whilst compiling %s\n", argv[i+1]);
                return -1;
            }

            i += 2;
        } else if (strcmp(argv[i], "-e") == 0){
            if (execute(argv[i+1]) != 0){
                printf("error whilst executing %s\n", argv[i+1]);
                return -1;
            }

            i += 2;
        } else {
            printf("unkown arguement: %s\n", argv[i]);
            i += 1;
        }
    }

    printf("exit: program finished\n");

    return 0;
}

// hello