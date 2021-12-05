#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

struct msg {
    long id;
    int value[3][3];
};

void makeMatrix(int** matrix, int x, int y);

int  makeMessageQueue(key_t ipckey, int flag) {
    int msqid = msgget(ipckey, flag);
    if (msqid < 0) {
        perror("msgget()");
        exit(0);
    }
    return msqid;
}
void setMessage3By3(struct msg* m, int** matrix, int a, int b, int msgid) {
    m->id = msgid;
    for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
            m->value[k][l] = matrix[a + k][b + l];
        }
    }
}
void setMessage2By2(struct msg* m, int** matrix, int a, int b, int msgid) {
    m->id = msgid;
    for (int c = 0; c < 3; c++) {
        for (int d = 0; d < 3; d++) {
            m->value[c][d] = 0;
        }
    }
    for (int k = 0; k < 2; k++) {
        for (int l = 0; l < 2; l++) {
            m->value[k][l] = matrix[a + k][b + l];
        }
    }
}
void setMessage1By1(struct msg* m, int result, int msgid) {
    m->id = msgid;
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            m->value[a][b] = 0;
        }
    }
    m->value[0][0] = result;
}
void sendMessage(int msqid, struct msg* ptr, size_t nbytes, int flag) {
    if (msgsnd(msqid, ptr, nbytes, flag) == -1) {
        perror("msgnd()");
        exit(0);
    }
}
void receiveMessage(int msqid, struct msg* ptr, size_t nbytes, long type, int flag) {
    if (msgrcv(msqid, ptr, nbytes, type, flag) == -1) {
        perror("msgrcv()");
        exit(0);
    }
}
int  convolutional(int arr[3][3]) {
    int result = 0;
    int filter[3][3] = { {-1,-1,-1},{-1,8,-1},{-1,-1,-1} };
    for (int g = 0; g < 3; g++) {
        for (int h = 0; h < 3; h++) {
            result += arr[g][h] * filter[g][h];
        }
    }
    return result;
}
int maxPooling(int arr[3][3]) {
    int result = arr[0][0];
    for (int c = 0; c < 2; c++) {
        for (int d = 0; d < 2; d++) {
            if (result < arr[c][d])result = arr[c][d];
        }
    }
    return result;
}

int main(int argc, char* argv[]) {

    int size = atoi(argv[1]);
    int x = size;
    int y = size;
    int** matrix;
    int** receive_matrix;
    int** result_array;
    key_t ipckey;
    int mqdes;
    int send_buf_len1, receive_buf_len2, send_buf_len3, receive_buf_len4;
    struct msg send_msg1, receive_msg2, send_msg3, receive_msg4;
    pid_t pid1[(x - 2) * (y - 2)];
    pid_t pid2[(x - 2) * (y - 2) / 4];

    send_buf_len1 = sizeof(send_msg1.value);
    send_buf_len3 = sizeof(send_msg3.value);

    receive_buf_len2 = sizeof(receive_msg2.value);
    receive_buf_len4 = sizeof(receive_msg4.value);

    ipckey = ftok("./", 2020);
    mqdes = makeMessageQueue(ipckey, IPC_CREAT | 0600);

    matrix = (int**)malloc(sizeof(int*) * x);
    for (int i = 0; i < x; i++) {
        matrix[i] = (int*)malloc(sizeof(int) * y);
    }
    receive_matrix = (int**)malloc(sizeof(int*) * (x - 2));
    for (int i = 0; i < (x - 2); i++) {
        receive_matrix[i] = (int*)malloc(sizeof(int) * (y - 2));
    }
    result_array = (int**)malloc(sizeof(int*) * (x - 2) / 2);
    for (int i = 0; i < (x - 2) / 2; i++) {
        result_array[i] = (int*)malloc(sizeof(int) * (y - 2) / 2);
    }

    makeMatrix(matrix, x, y);

    int msg_id1 = 1;
    int msg_id2 = 1;
    int pid_i1 = 0;
    for (int i = 0; i < (x - 2); i++) {
        for (int j = 0; j < (y - 2); j++) {
            setMessage3By3(&send_msg1, matrix, i, j, msg_id1);
            sendMessage(mqdes, &send_msg1, send_buf_len1, 0);
            if ((pid1[pid_i1] = fork()) == 0) {
                struct msg receive_msg1, send_msg2;
                int receive_buf_len1, send_buf_len2;
                receive_buf_len1 = sizeof(receive_msg1.value);
                send_buf_len2 = sizeof(send_msg2.value);

                receiveMessage(mqdes, &receive_msg1, receive_buf_len1, msg_id1, 0);
                int result = convolutional(receive_msg1.value);

                setMessage1By1(&send_msg2, result, msg_id2);
                sendMessage(mqdes, &send_msg2, send_buf_len2, 0);

                exit(1);
            }
            waitpid(pid1[pid_i1], NULL, 0);
            receiveMessage(mqdes, &receive_msg2, receive_buf_len2, msg_id2, 0);
            receive_matrix[i][j] = receive_msg2.value[0][0];
            msg_id1++;
            msg_id2++;
            pid_i1++;
        }
    }

    int msg_id3 = 1;
    int pid_i2 = 0;
    int msg_id4 = 1;
    for (int i = 0; i < (x - 2); i += 2) {
        for (int j = 0; j < (y - 2); j += 2) {
            setMessage2By2(&send_msg3, receive_matrix, i, j, msg_id3);
            sendMessage(mqdes, &send_msg3, send_buf_len3, 0);
            if ((pid2[pid_i2] = fork()) == 0) {
                struct msg receive_msg3, send_msg4;
                int receive_buf_len3, send_buf_len4;
                receive_buf_len3 = sizeof(receive_msg3.value);
                send_buf_len4 = sizeof(send_msg4.value);

                receiveMessage(mqdes, &receive_msg3, receive_buf_len3, msg_id3, 0);

                int result = maxPooling(receive_msg3.value);

                setMessage1By1(&send_msg4, result, msg_id4);
                sendMessage(mqdes, &send_msg4, send_buf_len4, 0);

                exit(1);
            }
            waitpid(pid2[pid_i2], NULL, 0);
            receiveMessage(mqdes, &receive_msg4, receive_buf_len4, msg_id4, 0);
            result_array[(i / 2)][(j / 2)] = receive_msg4.value[0][0];
            msg_id3++;
            msg_id4++;
            pid_i2++;
        }
    }

    for (int i = 0; i < (x - 2) / 2; i++) {
        for (int j = 0; j < (y - 2) / 2; j++) {
            printf("%d ", result_array[i][j]);
        }
    }

    for (int i = 0; i < x; i++) {
        free(matrix[i]);
    }
    free(matrix);
    for (int i = 0; i < (x - 2); i++) {
        free(receive_matrix[i]);
    }
    free(receive_matrix);
    for (int i = 0; i < (x - 2) / 2; i++) {
        free(result_array[i]);
    }
    free(result_array);

    msgctl(mqdes, IPC_RMID, 0);

    return 0;

}
