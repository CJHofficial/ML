#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

void makeMatrix(int** matrix, int x, int y);
struct messages {
    long id;
    int entry[3][3];
};
void makeMatrix3by3(struct messages* m, int** matrix, int i, int j, int msgid) {
    m->id = msgid;
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            m->entry[a][b] = matrix[i + a][j + b];
        }
    }
}
void makeMatrix2by2(struct messages* m, int** matrix, int i, int j, int msgid) {
    m->id = msgid;
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            m->entry[a][b] = 0;
        }
    }
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            m->entry[a][b] = matrix[i + a][i + b];
        }
    }
}
void makeMatrix1by1(struct messages* m, int k, int msgid) {
    m->id = msgid;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m->entry[i][j] = 0;
        }
    }
    m->entry[0][0] = k;
}
int Convolutional(int array[3][3]) {
    int sum = 0;
    int cmatrix[3][3] = { {-1,-1,-1},{-1,8,-1},{-1,-1,-1} };
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            sum += array[a][b] * cmatrix[a][b];
        }
    }
    return sum;
}
int Pooling(int array[3][3]) {
    int sum = array[0][0];
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            if (sum < array[a][b])
                sum = array[a][b];
        }
    }
    return sum;
}

int main(int argc, char* argv[]) {

    int x = atoi(argv[1]);
    int y = atoi(argv[1]);

    int** matrix;
    int** rematrix;
    int** finalmatrix;

    matrix = (int**)malloc(sizeof(int*) * x);
    for (int a = 0; a < x; a++) {
        matrix[a] = (int*)malloc(sizeof(int*) * y);
    }

    rematrix = (int**)malloc(sizeof(int*) * (x - 2));
    for (int a = 0; a < (x - 2); a++) {
        rematrix[a] = (int*)malloc(sizeof(int) * (y - 2));
    }

    finalmatrix = (int**)malloc(sizeof(int*) * ((x - 2) / 2));
    for (int a = 0; a < ((x - 2) / 2); a++) {
        finalmatrix[a] = (int*)malloc(sizeof(int) * ((y - 2) / 2));
    }
    makeMatrix(matrix, x, y);
    struct messages sm1, sm2, rm1, rm2;
    int sbuf_len1 = sizeof(sm1.entry);
    int sbuf_len2 = sizeof(sm2.entry);
    int rbuf_len1 = sizeof(rm1.entry);
    int rbuf_len2 = sizeof(rm2.entry);

    pid_t pid0[(x - 2) * (y - 2)];
    pid_t pid1[((x - 2) * (y - 2)) / 4];
    key_t ipckey;
    ipckey = ftok("./", 1997);
    int mqid = msgget(ipckey, IPC_CREAT | 0600);
    if (mqid < 0) {
        perror("msgget()");
        exit(0);
    }
    int mqdes = mqid;
    int msgid1 = 1;
    int msgid2 = 1;
    int pid3 = 0;
    for (int a = 0; a < (x - 2); a++) {
        for (int b = 0; b < (y - 2); b++) {
            makeMatrix3by3(&sm1, matrix, a, b, msgid1);
            if (msgsnd(mqdes, &sm1, sbuf_len1, 0) == -1) {
                perror("msgsnd()");
                exit(0);
            }//sending
            if ((pid0[pid3] = fork()) == 0) {
                struct messages sm3, rm3;
                int sbuf_len3 = sizeof(sm3.entry);
                int rbuf_len3 = sizeof(rm3.entry);
                if (msgrcv(mqdes, &rm3, rbuf_len3, msgid1, 0) == -1) {
                    perror("msgrcv()");
                    exit(0);
                }//receiving
                int sum = Convolutional(rm3.entry);
                makeMatrix1by1(&sm3, sum, msgid2);
                if (msgsnd(mqdes, &sm3, sbuf_len3, 0) == -1) {
                    perror("msgsnd()");
                    exit(0);
                }//sending
                exit(1);
            }
            waitpid(pid0[pid3], NULL, 0);
            if (msgrcv(mqdes, &rm1, rbuf_len1, msgid2, 0) == -1) {
                perror("msgrcv()");
                exit(0);
            }//receiving
            rematrix[a][b] = rm1.entry[0][0];;
            pid3++;
            msgid1++;
            msgid2++;
        }
    }

    int msgid3 = 1;
    int msgid4 = 1;
    int pid4 = 0;
    for (int a = 0; a < (x - 2); a += 2) {
        for (int b = 0; b < (y - 2); b += 2) {
            makeMatrix2by2(&sm2, rematrix, a, b, msgid3);
            if (msgsnd(mqdes, &sm2, sbuf_len2, 0) == -1) {
                perror("msgsnd()");
                exit(0);
            }//sending
            if ((pid1[pid4] = fork()) == 0) {
                struct messages sm4, rm4;
                int sbuf_len4 = sizeof(sm4.entry);
                int rbuf_len4 = sizeof(rm4.entry);
                if (msgrcv(mqdes, &rm4, rbuf_len4, msgid3, 0) == -1) {
                    perror("msgrcv()");
                    exit(0);
                }//receiving       
                int sum = Pooling(rm4.entry);
                makeMatrix1by1(&sm4, sum, msgid4);
                if (msgsnd(mqdes, &sm4, sbuf_len4, 0) == -1) {
                    perror("msgsnd()");
                    exit(0);
                }//sending
                exit(1);
            }
            waitpid(pid1[pid4], NULL, 0);
            if (msgrcv(mqdes, &rm2, rbuf_len2, msgid4, 0) == -1) {
                perror("msgrcv()");
                exit(0);
            }//receiving
            finalmatrix[(a / 2)][(b / 2)] = rm2.entry[0][0];
            pid4++;
            msgid3++;
            msgid4++;
        }
    }

    for (int a = 0; a < ((x - 2) / 2); a++) {
        for (int b = 0; b < ((y - 2) / 2); b++) {
            printf("%d ", finalmatrix[a][b]);
        }
    }
    for (int a = 0; a < x; a++) {
        free(matrix[a]);
    }
    free(matrix);
    for (int a = 0; a < (x - 2); a++) {
        free(rematrix[a]);
    }
    free(rematrix);
    for (int a = 0; a < ((x - 2) / 2); a++) {
        free(finalmatrix[a]);
    }
    free(finalmatrix);
    msgctl(mqdes, IPC_RMID, 0);

    return 0;
}