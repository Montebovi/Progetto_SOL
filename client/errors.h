
#ifndef CLIENT_ERRORS_H
#define CLIENT_ERRORS_H

#define ERR_NO_ERROR                (0)
#define ERR_CONNECTION_NOT_OPEN     (1)
#define ERR_CONNECTION_NOT_CLOSED   (2)

#define ERR_RECEIVE_DATA            (3)
#define ERR_OUT_OF_MEMORY           (4)
#define ERR_SEND_DATA               (5)
#define ERR_DIR_NOT_SPECIFIED       (6)
#define ERR_INVALID_NUMBER_OF_ARGUMENTS (7)
#define ERR_DIRECTORY_NOT_FOUND     (8)
#define ERR_OPEN_FAILURE            (9)
#define ERR_READ_ERROR              (10)
#define ERR_FILE_CLOSE_ERROR        (11)
#define ERR_LOCK_FILE_FAILURE       (12)
#define ERR_UNLOCK_FILE_FAILURE     (13)
#define ERR_FILE_NOT_REMOVED        (14)

#define ERR_RET_VAL_ERROR           (99)

void manageRetValError(int retval);

#endif //CLIENT_ERRORS_H
