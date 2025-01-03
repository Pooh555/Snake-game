#ifndef SNAKE_H_INCLUDED
#define SNAKE_H_INCLUDED

#define MAX_SNAKE_LEN 100
#define MAX_LIFE 3
#define SNAKE_HEAD ACS_DIAMOND
#define SNAKE_BODY 'O'
#define S_HEAD 0
#define S_TAIL (SnakeLen-1)
#define FOOD '$'
#define FOOD_BONUS 100;
#define MAX_AUTO_TIMER 30
#define MIN_AUTO_TIMER 3

#define SNAKE_COLOR COLOR_PAIR(2)
#define MSG_COLOR COLOR_PAIR(3)
#define BORDER_COLOR COLOR_PAIR(1)
#define FOOD_COLOR COLOR_PAIR(4)

typedef struct {
   int tp_x, tp_y;
   int bt_x, bt_y;
} Area;

typedef enum {F_EMPTY, F_FULL} FoodStat;
typedef struct {
   int x,y;
   FoodStat stat;
} FoodCell;

typedef enum {S_LEFT='<', S_RIGHT='>', S_UP='A', S_DOWN='V'} Direction;
typedef struct {
   int x,y;
   Direction go;
} SnakeCell;

extern void InitGame(void);
extern void AdjustScreen(void);
extern void ClearPlayArea(void);
extern void ClearMsgArea(void);
extern void DrawBackground(void);
extern void ExitGame(void);
extern void AddSnakeTail(void);
extern void NewSnake(void);
extern void NewFood(void);
extern int FoodEaten();
extern int KbHit(void);
extern void CheckKey(void);
extern void Bend(Direction dir);
extern void ClearSnakeTail(void);
extern void ShowSnake(void);
extern void ShowFood(void);
extern int HitBody(Direction dir);
extern int HitBorder(Direction dir);
extern void MoveSnakeBody(void);
extern void MoveSnakeHead(Direction dir);
extern void MoveSnake(Direction dir);
extern void PauseGame(void);
extern void EndGame(void);
extern void ShowMark(void);
extern void GameAction(void);

//------------------------------- Quiz Subsystem --------------------------------------
#define QUIZFILE "quiz.txt"
#define MAX_QUIZ 50
#define QUESTION_LEN 200
#define CHOICE_LEN 100
#define MAX_CHOICE 3

typedef struct {
    char question[QUESTION_LEN];
    char choice[MAX_CHOICE][CHOICE_LEN];
    int answer;
} Quiz;

extern int ReadLine(char *str, int size, FILE *fp );
extern int ReadQuizFile(char *fname);
extern void AskQuiz(void);

#endif // SNAKE_H_INCLUDED
