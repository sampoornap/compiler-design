#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TAC {
    char op[4];     
    char arg1[32];  //First argument
    char arg2[32];  //Second argument (if any)
    char result[32];//Result variable
    struct TAC *next;
    char label[32]; //Label if any
} TAC;

TAC *codeHead = NULL;
TAC *codeTail = NULL;

void addInstruction(const char *op, const char *arg1, const char *arg2, const char *result, const char *label) {
    TAC *newInstr = (TAC *)malloc(sizeof(TAC));
    if (newInstr == NULL) {
        fprintf(stderr, "Memory allocation failed for TAC instruction\n");
        exit(1);
    }
    strncpy(newInstr->op, op, sizeof(newInstr->op));
    strncpy(newInstr->arg1, arg1 ? arg1 : "", sizeof(newInstr->arg1));
    strncpy(newInstr->arg2, arg2 ? arg2 : "", sizeof(newInstr->arg2));
    strncpy(newInstr->result, result ? result : "", sizeof(newInstr->result));
    strncpy(newInstr->label, label ? label : "", sizeof(newInstr->label));
    newInstr->next = NULL;
    
    if (codeHead == NULL) {
        codeHead = codeTail = newInstr;
    } else {
        codeTail->next = newInstr;
        codeTail = newInstr;
    }
}

void readTACFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file for reading TAC");
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char label[32] = "", op[4] = "", arg1[32] = "", arg2[32] = "", result[32] = "";
        
        if (line[0] == 'L') { // Label line
            sscanf(line, "%s", label);
            addInstruction("", "", "", "", label);
        } 
        else if (strstr(line, "goto") || strstr(line, "ifFalse")) {
            addInstruction("", "", "", "", line);
           }
        
        else {
            int numArgs = sscanf(line, "%s = %s %s %s", result, arg1, op, arg2);
            if (numArgs == 4) {
                addInstruction(op, arg1, arg2, result, "");
            } else if (numArgs == 2) {
                addInstruction(":=", arg1, NULL, result, "");
            }
        }
    }
    fclose(file);
}

void writeOptimizedTACToFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing optimized TAC");
        exit(1);
    }

    TAC *current = codeHead;
    while (current != NULL) {
        if (strlen(current->label) > 0) {
            fprintf(file, "%s\n", current->label);
        } else if (strlen(current->arg2) > 0) {
            fprintf(file, "%s = %s %s %s\n", current->result, current->arg1, current->op, current->arg2);
        } else if (strlen(current->arg1) > 0) {
            fprintf(file, "%s = %s\n", current->result, current->arg1);
        }
        current = current->next;
    }
    fclose(file);
}

void constantFolding() {
    TAC *current = codeHead;
    while (current != NULL) {
        if (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
            strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0) {
            int left, right;
            if (sscanf(current->arg1, "%d", &left) == 1 && sscanf(current->arg2, "%d", &right) == 1) {
                int result;
                if (strcmp(current->op, "+") == 0) result = left + right;
                if (strcmp(current->op, "-") == 0) result = left - right;
                if (strcmp(current->op, "*") == 0) result = left * right;
                if (strcmp(current->op, "/") == 0) result = left / right;
                sprintf(current->arg1, "%d", result);
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            }
        }
        current = current->next;
    }
}

void copyPropagation() {
    TAC *current = codeHead;
    while (current != NULL) {
        if (current->op[0] == '\0') {  
            TAC *inner = current->next;
            while (inner != NULL) {
                if (strcmp(inner->arg1, current->result) == 0) {
                    strcpy(inner->arg1, current->arg1);
                }
                if (strcmp(inner->arg2, current->result) == 0) {
                    strcpy(inner->arg2, current->arg1);
                }
                inner = inner->next;
            }
        }
        current = current->next;
    }
}

void constantPropagation() {
    TAC *current = codeHead;
    while (current != NULL) {
        if (current->op[0] == '\0') {  // Simple assignment (constant propagation)
            int value;
            if (sscanf(current->arg1, "%d", &value) == 1) {
                TAC *inner = current->next;
                while (inner != NULL) {
                    if (strcmp(inner->arg1, current->result) == 0) {
                        sprintf(inner->arg1, "%d", value);
                    }
                    if (strcmp(inner->arg2, current->result) == 0) {
                        sprintf(inner->arg2, "%d", value);
                    }
                    inner = inner->next;
                }
            }
        }
        current = current->next;
    }
}

void commonSubexpressionElimination() {
    TAC *current = codeHead;
    while (current != NULL) {
        TAC *inner = current->next;
        while (inner != NULL) {
            if (strcmp(current->op, inner->op) == 0 &&
                strcmp(current->arg1, inner->arg1) == 0 &&
                strcmp(current->arg2, inner->arg2) == 0) {
                //Replace inner result with current result
                TAC *temp = inner->next;
                while (temp != NULL) {
                    if (strcmp(temp->arg1, inner->result) == 0) {
                        strcpy(temp->arg1, current->result);
                    }
                    if (strcmp(temp->arg2, inner->result) == 0) {
                        strcpy(temp->arg2, current->result);
                    }
                    temp = temp->next;
                }
            }
            inner = inner->next;
        }
        current = current->next;
    }
}

void algebraicSimplification() {
    TAC *current = codeHead;
    while (current != NULL) {
        if (strcmp(current->op, "+") == 0) {
            if (strcmp(current->arg1, "0") == 0) {
                strcpy(current->arg1, current->arg2);
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            } else if (strcmp(current->arg2, "0") == 0) {
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            }
        } else if (strcmp(current->op, "-") == 0) {
            if (strcmp(current->arg2, "0") == 0) {
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            }
        } else if (strcmp(current->op, "*") == 0) {
            if (strcmp(current->arg1, "0") == 0 || strcmp(current->arg2, "0") == 0) {
                strcpy(current->arg1, "0");
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            } else if (strcmp(current->arg1, "1") == 0) {
                strcpy(current->arg1, current->arg2);
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            } else if (strcmp(current->arg2, "1") == 0) {
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            }
        } else if (strcmp(current->op, "/") == 0) {
            if (strcmp(current->arg2, "1") == 0) {
                current->op[0] = '\0';
                current->arg2[0] = '\0';
            }
        }
        current = current->next;
    }
}

void optimizeTAC() {
    constantFolding();
    copyPropagation();
    constantPropagation();
    commonSubexpressionElimination();
    algebraicSimplification();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input tac file> <output optimized tac file>\n", argv[0]);
        return 1;
    }

    readTACFromFile(argv[1]);
    optimizeTAC();
    writeOptimizedTACToFile(argv[2]);

    return 0;
}
