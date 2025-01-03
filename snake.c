#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "snake.h"

int GameExit;            // 0 = Continue, 1 = Exit
int SnakeLen;            // Length of the snake
int SnakeLife = MAX_LIFE;
int SnakeDead;          // 0 = Alive, 1 = Dead
int ForceMove;          // 0 = No Move, 1 = Move the snake 1 position
Direction NextMove;     // Direction of snake
int Mark;
Area MsgArea, PlayArea;
SnakeCell Snake[MAX_SNAKE_LEN];
FoodCell Food;

int main (void)
{
   InitGame();
   PauseGame();

   while (!GameExit)
   {
      CheckKey();
      GameAction();
   }

   ExitGame();
   return 0;
}

void InitGame(void)
{
   initscr();                 // Start curses mode
   raw();                     // Line buffering disabled
   keypad(stdscr, TRUE);      // We get F1, F2 etc..
   nodelay(stdscr, TRUE);     // Don't wait for key press
   nonl();                    // Don't convert CR to CR/LF
   noecho();                  // Don't echo() while we do getch
   curs_set(0);               // Hide cursor

   if (has_colors())
   {
      start_color();

      /* Simple color assignment, often all we need.  Color pair 0 cannot
         be redefined.  This example uses the same value for the color
         pair as for the foreground color, though of course that is not
         necessary: */
      init_pair(1, COLOR_BLACK, COLOR_RED);
      init_pair(2, COLOR_BLUE,  COLOR_CYAN);
      init_pair(3, COLOR_BLACK, COLOR_CYAN);
      init_pair(4, COLOR_WHITE, COLOR_BLACK);
   }

   AdjustScreen();
   DrawBackground();

   srand(time(NULL));            // use current time as seed for random generator
   NewSnake();                   // A snake was born
   Mark = 0;                     // Reset scores
   ReadQuizFile(QUIZFILE);       // Load quiz data from a text file;
}

void AdjustScreen(void)
{
   PlayArea.tp_x = 1;
   PlayArea.tp_y = 1;
   PlayArea.bt_x = COLS-2;
   PlayArea.bt_y = LINES-10;
   MsgArea.tp_x = 1;
   MsgArea.tp_y = PlayArea.bt_y+2;
   MsgArea.bt_x = COLS-2;
   MsgArea.bt_y = LINES-2;
}

void ClearPlayArea(void)
{
   attrset(SNAKE_COLOR);
   for (int i=PlayArea.tp_y; i<=PlayArea.bt_y; ++i)
      mvhline(i,PlayArea.tp_x,' ',PlayArea.bt_x-PlayArea.tp_x+1);
}

void ClearMsgArea(void)
{
   attrset(MSG_COLOR);
   for (int i=MsgArea.tp_y; i<=MsgArea.bt_y; ++i)
      mvhline(i,MsgArea.tp_x,' ',MsgArea.bt_x-MsgArea.tp_x+1);
}

void DrawBackground(void)
{
//   clear();
   attrset(BORDER_COLOR);
   wborder(stdscr,' ',' ',' ',' ',' ',' ',' ',' ');
   mvhline(PlayArea.bt_y+1,PlayArea.tp_x,' ',PlayArea.bt_x-PlayArea.tp_x+1);
   attron(A_BOLD|A_REVERSE);
   mvaddstr(0,(COLS-12)/2," Snake Game ");

   ClearPlayArea();
   ClearMsgArea();

   attrset(MSG_COLOR);
   mvaddstr(MsgArea.tp_y,MsgArea.tp_x+1,"Mark:");
   mvaddstr(MsgArea.tp_y,MsgArea.bt_x-20,"Lifes:");
   refresh();
}

void ShowMark(void)
{
   attrset(MSG_COLOR);
   move(MsgArea.tp_y,MsgArea.tp_x+7);
   printw("%-5d",Mark);
   move(MsgArea.tp_y,MsgArea.bt_x-13);
   printw("%d",SnakeLife);
   refresh();
}

void NewSnake(void)
{

   Snake[S_HEAD].x = (PlayArea.bt_x-PlayArea.tp_x)/2;
   Snake[S_HEAD].y = (PlayArea.bt_y-PlayArea.tp_y)/2;
   Snake[S_HEAD].go = S_UP;

   SnakeLen = 1;
   SnakeDead = 0;
}

void AddSnakeTail(void)
{
   if (SnakeLen < MAX_SNAKE_LEN)
   {
      Snake[SnakeLen] = Snake[S_TAIL];
      ++SnakeLen;
   }
}

void ShowSnake(void)
{
   int i=S_HEAD;

   attrset(SNAKE_COLOR);

   for (i=S_TAIL; i>S_HEAD; --i)
     mvaddch(Snake[i].y, Snake[i].x, SNAKE_BODY );

   mvaddch(Snake[S_HEAD].y, Snake[S_HEAD].x, Snake[S_HEAD].go);

   refresh();
}

void ClearSnakeTail(void)
{
   attrset(SNAKE_COLOR);
   mvaddch(Snake[S_TAIL].y, Snake[S_TAIL].x, ' ');
}

void MoveSnakeBody(void)
{
   for (int i=S_TAIL; i>S_HEAD; --i)
      Snake[i]=Snake[i-1];
}

void MoveSnakeHead(Direction dir)
{
   switch (dir)
   {
   case S_UP:
      Snake[S_HEAD].y--;
      break;
   case S_DOWN:
      Snake[S_HEAD].y++;
      break;
   case S_LEFT:
      Snake[S_HEAD].x--;
      break;
   case S_RIGHT:
      Snake[S_HEAD].x++;
      break;
   }
   Snake[S_HEAD].go = dir;
}

void MoveSnake(Direction dir)
{
   if (HitBorder(dir) || HitBody(dir))
   {
      SnakeDead = 1;
      return;
   }

   ClearSnakeTail();
   MoveSnakeBody();
   MoveSnakeHead(dir);
   ShowSnake();
}

void ShowFood(void)
{
   if (Food.stat==F_FULL)
   {
      attrset(FOOD_COLOR);
      mvaddch(Food.y, Food.x, FOOD );
      attrset(SNAKE_COLOR);
      refresh();
   }
}

void NewFood(void)
{
   if (Food.stat == F_EMPTY)
   {
      Food.x = PlayArea.tp_x + rand()%(PlayArea.bt_x-PlayArea.tp_x);
      Food.y = PlayArea.tp_y + rand()%(PlayArea.bt_y-PlayArea.tp_y);
      Food.stat = F_FULL;
   }
   ShowFood();
}

int FoodEaten()
{
   if (Food.x==Snake[S_HEAD].x && Food.y==Snake[S_HEAD].y)
   {
      Food.stat = F_EMPTY;
      Mark += FOOD_BONUS;
      ShowMark();
      return 1;
   }
   return 0;
}

void ExitGame(void)
{
   endwin();
}

void Bend(Direction dir)
{
   ForceMove = 1;
   NextMove = dir;
}

int HitBody(Direction dir)
{
   int x=0,y=0;
   int hit=0;

   switch (dir)
   {
   case S_UP:
      x = Snake[S_HEAD].x;
      y = Snake[S_HEAD].y-1;
      break;
   case S_DOWN:
      x = Snake[S_HEAD].x;
      y = Snake[S_HEAD].y+1;
      break;
   case S_LEFT:
      x = Snake[S_HEAD].x-1;
      y = Snake[S_HEAD].y;
      break;
   case S_RIGHT:
      x = Snake[S_HEAD].x+1;
      y = Snake[S_HEAD].y;
      break;
   }

   for (int i=S_HEAD+1; i<SnakeLen; ++i)
      if (x==Snake[i].x && y==Snake[i].y)
      {
         hit = 1;
         break;
      }
   return hit;
}

int HitBorder(Direction dir)
{
   int x, y;

   x = Snake[S_HEAD].x;
   y = Snake[S_HEAD].y;
   if ((dir==S_UP && y==PlayArea.tp_y) || (dir==S_DOWN && y==PlayArea.bt_y) || (dir==S_LEFT && x==PlayArea.tp_x) || (dir==S_RIGHT && x==PlayArea.bt_x))
      return 1;

   return 0;
}

void PauseGame(void)
{
   attrset(MSG_COLOR);
   mvaddstr(MsgArea.tp_y+1,MsgArea.tp_x+1,"Press any key to play!!!");
   nodelay(stdscr, FALSE);
   getch();
   mvhline(MsgArea.tp_y+1,MsgArea.tp_x,' ',MsgArea.bt_x-MsgArea.tp_x+1);
   nodelay(stdscr, TRUE);
   refresh();
}

void EndGame(void)
{
   int ch;

   attrset(MSG_COLOR);
   mvaddstr(MsgArea.tp_y+1,MsgArea.tp_x+1,"Press Y to exit, N to continue.");
   nodelay(stdscr, FALSE);

   do
   {
      ch = toupper(getch());

   } while (ch!='Y' && ch!='N');

   if (ch=='Y')
      GameExit = 1;
   else
   {
      mvhline(MsgArea.tp_y+1,MsgArea.tp_x,' ',MsgArea.bt_x-MsgArea.tp_x+1);
      nodelay(stdscr, TRUE);
      refresh();
   }
}

void ResetGame(void)
{
   ClearPlayArea();
   NewSnake();
}

void RestartGame(void)
{
   EndGame();
   if (!GameExit)
   {
      ResetGame();
      Mark = 0;
      SnakeLife = MAX_LIFE;
      ShowMark();
   }
}

int KbHit(void)
{
   int ch = getch();

   if (ch != ERR)
   {
      ungetch(ch);
      return 1;
   } else
      return 0;
}

void CheckKey(void)
{
   int ch;

   if (!KbHit())
      return;

   ch = getch();
   switch (ch)
   {
      case KEY_UP:
         if (Snake[S_HEAD].go!=S_DOWN) Bend(S_UP);
         break;
      case KEY_DOWN:
         if (Snake[S_HEAD].go!=S_UP) Bend(S_DOWN);
         break;
      case KEY_LEFT:
         if (Snake[S_HEAD].go!=S_RIGHT) Bend(S_LEFT);
         break;
      case KEY_RIGHT:
         if (Snake[S_HEAD].go!=S_LEFT) Bend(S_RIGHT);
         break;
      case ' ':
         PauseGame();
         break;
      case 'q':
      case 'Q':
         EndGame();
         break;
    }
}

void GameAction(void)
{
   static int FoodTimer=0;
   static int SnakeTimer=0;
   static int AutoTimer=0;
   static int MaxAutoTimer=MAX_AUTO_TIMER;

   usleep(20000);

   if (SnakeDead)
   {
      --SnakeLife;
      ShowMark();
      if (SnakeLife>0)
      {
         //PauseGame();
         AskQuiz();
         ResetGame();
      }
      else
      {
         RestartGame();
      }
      MaxAutoTimer = MAX_AUTO_TIMER;
   }
   else
   {
      Mark++;
      ShowMark();
   }

   if (AutoTimer==0 || ForceMove)
   {
      if (ForceMove)
      {
         MoveSnake(NextMove);
         ForceMove = 0;
      }
      else
      {
         MoveSnake(Snake[S_HEAD].go);
         AutoTimer = MaxAutoTimer;
      }
   }
   else
      AutoTimer--;

   if (FoodTimer==0 || FoodEaten())
   {
      NewFood();
      FoodTimer = 15;
   }
   else
      FoodTimer--;

   if (SnakeLen<MAX_SNAKE_LEN)
   {
      if (SnakeTimer==0)
      {
         AddSnakeTail();
         SnakeTimer = 20;
         if (MaxAutoTimer >= MIN_AUTO_TIMER)
            MaxAutoTimer--;
      }
      else
         SnakeTimer--;
   }
}

//------------------------------- Quiz Subsystem --------------------------------------

Quiz QuizArr[MAX_QUIZ];
int QuizNumber=0;

int ReadLine(char *str, int size, FILE *fp )
{
   if ( fgets(str, size, fp) == NULL ) // read a line from a file and copy to a string
      return -1;

   size = strcspn(str, "\n"); // Check position of '\n'
   str[size] = 0;             // Remove '\n'
   return size;
}

int ReadQuizFile(char *fname)
{
   FILE *fp;
   char ans[5];
   Quiz *quiz = QuizArr;

   fp = fopen(fname,"r");
   if (fp!=NULL)
   {
      while (!feof(fp) && QuizNumber<MAX_QUIZ)
      {
         if ( ReadLine((*quiz).question, QUESTION_LEN, fp) < 0 )
            break;
         if ( ReadLine((*quiz).choice[0], CHOICE_LEN, fp) < 0 )
            break;
         if ( ReadLine((*quiz).choice[1], CHOICE_LEN, fp) < 0 )
            break;
         if ( ReadLine((*quiz).choice[2], CHOICE_LEN, fp) < 0 )
            break;
         if ( ReadLine( ans, 5, fp) < 0 )
            break;
         (*quiz).answer = atoi(ans);
         ++QuizNumber;
         ++quiz;
      }
      fclose(fp);
      return 1;
   }
   return 0;
}

void AskQuiz()
{
   int num;
   int ch;

   num = rand() % QuizNumber;

   attrset(MSG_COLOR);
   mvaddstr(MsgArea.tp_y+1,MsgArea.tp_x+1,QuizArr[num].question);
   mvaddstr(MsgArea.tp_y+2,MsgArea.tp_x+1,QuizArr[num].choice[0]);
   mvaddstr(MsgArea.tp_y+3,MsgArea.tp_x+1,QuizArr[num].choice[1]);
   mvaddstr(MsgArea.tp_y+4,MsgArea.tp_x+1,QuizArr[num].choice[2]);

   nodelay(stdscr, FALSE);
   do
   {
     ch = getch()-'0';
   } while( ch<1 || ch>3 );

   if (ch==QuizArr[num].answer)
   {
     mvaddstr(MsgArea.tp_y+5,MsgArea.tp_x+1,"Congratulation!!! - Extra Life");
     ++SnakeLife;
   }
   else
     mvaddstr(MsgArea.tp_y+5,MsgArea.tp_x+1,"Sorry!!! - Try again");

   getch();

   for (int i=MsgArea.tp_y+1; i<=MsgArea.bt_y; ++i)
      mvhline(i,MsgArea.tp_x,' ',MsgArea.bt_x-MsgArea.tp_x+1);

   nodelay(stdscr, TRUE);
   refresh();
}

