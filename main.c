#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define ScreenHeight 54
#define ScreenWidth 190
#define UpLimit 7
#define DownLimit 48
#define RightLimit 160
#define LeftLimit 30

#define TIMELIMIT 0.2
#define MaximumRoomW 15
#define MinimumRoomW 9
#define MaximumRoomL 10
#define MinimumRoomL 5
#define MaxRoomCount 15
#define MinRoomCount 8
#define PlayerSymbol '@'
#define FOOD_SYMBOL 'H'
#define SPELL_SYMBOL '%'
#define TREASURE 100
// Droprates
#define FOOD_DROP 100
#define GOLD_DROP 30
#define WEAPON_DROP 800
#define SpellDrop 400
// color
#define YELLOW 1
#define RED 2
#define BLUE 3
#define Cyan 4
#define GREEN 5
#define ORANGE 6
#define MAGENTA 7
#define PINK 8
#define PURPLE 9
#define BROWN 10
#define WHITE 11
#define BLACK 12

// weapons
#define HAND -1
#define MACE 0      // damage  5, damages all 8 blocks around the player
#define DAGGER 1    // damage 12, range 5, set 10
#define MAGICWAND 2 // damage 15, range 10, set 8
#define ARROW 3     // damage 5, range 5, set 20
#define SWORD 4     // damage 10. damages all 8 blocks around the player
#define DROPPEDARROW 5
#define DROPPEDWAND 6

// food
#define NORMALFOOD 0
#define FINEFOOD 1
#define MAGICFOOD 2
#define SPOILEDFOOD 3
#define EXPIRATION 100

// Monsters
#define MONSTER_SPAWN_RATE 600
#define DEAMON 'D'
#define FIRE_BREATHING_MONSTER 'F'
#define GIANT 'G'
#define SNAKE 'S'
#define UNDEAD 'U'

double Scarcity = 1;
int Level = 1;
int GameState = 1;
int Difficulty = 1; // 1: Easy 2: Hard.
int HeroColor = 1;  // 1: Yellow 2: Red 3: Blue.
int speed = 1;
int strength = 1;
int Music = 1;
char User[80];
int logged = 0;

typedef struct
{
    int weapninventory[5];
    int foodinventory[4];
    int spellinventory[3];
    int x;
    int y;
    int Level;
    int FullHP;
    int HP;
    int xp;
    int Gold;
    int PP;
    int FullEnergy;
    int Energy;
    int Speed;
    int Health;
    int Damage;
    int wielded;
    int score;
    int treasure;
} Player;

typedef struct
{
    char name[20];
    int x;
    int y;
} Button;

typedef struct
{
    int x1, y1;
    int x2, y2;
    int show;
    int type; // 1: Regular rooms.
} Room;

typedef struct
{
    int x;
    int y;
} Coordinate;

typedef struct
{
    int type; // 1: deamon 2: fire breathing monster 3: Giant 4: snake 5: undead
    int aggro;
    int x, y;
    int damage;
    int HP;
    int pp;
    char name;
    int dead;
} Monster;

typedef struct
{
    int index;
    int gold;
    int score;
    int gamesplayed;
    char name[50];
} Participant;

void DeleteElement(int size, int a[size], int e);
void SignOrLog();
void SignupPage();
void LoginPage();
int ValidateEmail(const char *email);
int ValidateUsername(const char *username);
int ValidatePassword(const char *password);
char *ReadFileLine(int line, int size, FILE *file);
int MainMenuSetup(int res);
void NewGame();
void InitPlayer(Player *player);
void PlayGame(Player *player, char **map1, Room **rooms1, int RoomCount1);
void wingame(Player *player);
void lose(Player *player);
void InitMonsterLevel(char **map, Monster **monster, int *MonsterCount);
Monster *InitMonster(char **map, int y, int x);
int sign(int x);
int CheckCoMonster(char **map, int y, int x);
int dif(int a);
void MonsterMovement(int MonsterAggroAge[4][100], int MonsterCount, Monster **monster, Player *player, char **map);
int Movement(char **map, char **VisibleMap, Player *player, Room **rooms, int RoomCount, int *ShowMap, int NormalFoodAge[50], int MagicFoodAge[50], int FineFoodAge[50], int input);
void kill(char **map, Monster *monster);
void Attack(char **map, Player *player, Monster **monster, int MonsterCount);
void Project(char **map, Player *player, Monster **monster, int MonsterCount, Room **rooms);
void LoadFoodInventory(Player *player, int NormalFoodAge[50], int MagicFoodAge[50], int FineFoodAge[50]);
void LoadSpellInventory(Player *player);
void LoadWeaponInventory(Player *player);
void CheckDoor(Room **rooms, char **map, int RoomCount, int y, int x);
int CheckCo(Player *player, char **map, int y, int x);
void MapInit(char ***map, Room **rooms, int RoomCount, Room *StairRoom);
Room *MakeRoom(char **map);
void DrawRoom(char **map, Room *room);
void ConnectRooms(char ***map, Room **rooms, Room *room1, Room *room2, int RoomCount, Room *StairRoom);
void DrawCorridor(char **map, int x, int y);
int SpawnPlayer(Room **rooms, int RoomCount, Player *player, char **map);
int SpawnStairs(char **map, Room **rooms, int RoomCount, int SpawnRoom);
void Settings();
void ScoreBoard();

void Exit();

Button *MakeButton(int y, int x, const char *name);
WINDOW *MakeWindow(Button *Button);

int main()
{
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    raw();
    keypad(stdscr, true);
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music *music1 = Mix_LoadMUS("music1.mp3");
    Mix_PlayMusic(music1, -1);
    SignOrLog();
    MainMenuSetup(0);
    srand(time(NULL));
    if (GameState == 0)
    {
        endwin();
        return 0;
    }

    endwin();
    return 0;
}

void DeleteElement(int size, int a[size], int e)
{
    for (int i = 0; i < size - 1; i++)
    {
        a[i] = a[i + 1];
    }
    a[size - 1] = 0;
    return;
}

void SignOrLog()
{
    clear();
    raw();
    curs_set(0);
    noecho();
    cbreak();
    start_color();

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    int Choice = 1;
    WINDOW *SignBox = newwin(3, 10, ScreenHeight / 2 - 5, ScreenWidth / 2 - 3);
    WINDOW *LogBox = newwin(3, 10, ScreenHeight / 2, ScreenWidth / 2 - 3);
    WINDOW *GuestBox = newwin(3, 10, ScreenHeight / 2 + 5, ScreenWidth / 2 - 3);
    refresh();
    mvwprintw(SignBox, 1, 1, "SIGN UP");
    mvwprintw(LogBox, 1, 1, "LOG IN");
    mvwprintw(GuestBox, 1, 1, "GUEST");

    while (1)
    {
        if (Choice == 1)
        {
            wbkgd(SignBox, COLOR_PAIR(1));
            wbkgd(LogBox, COLOR_PAIR(2));
            wbkgd(GuestBox, COLOR_PAIR(2));
        }
        else if (Choice == 2)
        {
            wbkgd(GuestBox, COLOR_PAIR(2));
            wbkgd(SignBox, COLOR_PAIR(2));
            wbkgd(LogBox, COLOR_PAIR(1));
        }
        else if (Choice == 3)
        {
            wbkgd(GuestBox, COLOR_PAIR(1));
            wbkgd(SignBox, COLOR_PAIR(2));
            wbkgd(LogBox, COLOR_PAIR(2));
        }
        wrefresh(GuestBox);
        wrefresh(LogBox);
        wrefresh(SignBox);
        refresh();
        int c = getch();
        if (c == KEY_UP || c == KEY_RIGHT)
        {
            if (Choice > 1)
            {
                Choice--;
            }
        }
        else if (c == KEY_DOWN || c == KEY_LEFT)
        {
            if (Choice < 3)
            {
                Choice++;
            }
        }
        if (c == '\n')
        {
            if (Choice == 1)
            {
                clear();
                SignupPage();
                return;
            }
            if (Choice == 2)
            {
                clear();
                LoginPage();
                return;
            }
            if (Choice == 3)
            {
                clear();
                return;
            }
        }
    }
}

int ValidateEmail(const char *email)
{
    const char *at = strchr(email, '@');
    if (!at)
    {
        return 0;
    }
    const char *dot = strrchr(at, '.');
    if (!dot)
    {
        return 0;
    }
    if (at == email || dot == at + 1 || *(dot + 1) == '\0')
    {
        return 0;
    }
    return 1;
}

int ValidateUsername(const char *username)
{

    FILE *UsernameFile = fopen("user data/usernames.txt", "r");
    char temp[33];

    while (fgets(temp, 33, UsernameFile))
    {
        temp[strlen(temp) - 1] = '\0';
        if (strcmp(temp, username) == 0)
        {
            return 0;
        }
    }
    return 1;
}

int ValidatePassword(const char *password)
{
    int check = 0;
    if (strlen(password) >= 7)
    {
        check++;
    }

    for (int i = 0; password[i]; i++)
    {
        if (password[i] >= 'a' && password[i] <= 'z')
        {
            check++;
            break;
        }
    }
    for (int i = 0; password[i]; i++)
    {
        if (password[i] >= 'A' && password[i] <= 'Z')
        {
            check++;
            break;
        }
    }
    for (int i = 0; password[i]; i++)
    {
        if (password[i] >= '0' && password[i] <= '9')
        {
            check++;
            break;
        }
    }
    if (check == 4)
    {
        return 1;
    }

    return 0;
}

void SignupPage()
{
    initscr();
    echo();
    curs_set(1);
    mvprintw(ScreenHeight / 2 - 6, ScreenWidth / 2 - 4, "Email address");
    mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 15, "Username (up to 32 characters)");
    mvprintw(ScreenHeight / 2 + 6, ScreenWidth / 2 - 15, "Password (7 - 32 charcaters)");
    refresh();
    char Username[32], Email[60], Password[32];

    WINDOW *EmailWindow = newwin(3, 50, (ScreenHeight / 2) - 4, ScreenWidth / 2 - 25);
    box(EmailWindow, 0, 0);
    WINDOW *UsernameWindow = newwin(3, 32, (ScreenHeight / 2) + 2, ScreenWidth / 2 - 13);
    box(UsernameWindow, 0, 0);
    WINDOW *PasswordWindow = newwin(3, 32, (ScreenHeight / 2) + 8, ScreenWidth / 2 - 13);
    box(PasswordWindow, 0, 0);

    refresh();
    wrefresh(UsernameWindow);
    wrefresh(PasswordWindow);
    wrefresh(EmailWindow);

    mvprintw(0, 0, "Press \"shift + q\" to go back. if you want to continue press any other key.");
    refresh();
    char c = getch();
    if (c == 'Q')
    {
        SignOrLog();
        return;
    }
    else
    {
        mvprintw(0, 0, "                                                                                             ");
        refresh();
    }

    mvwgetnstr(EmailWindow, 1, 1, Email, 50);
    while (!ValidateEmail(Email))
    {
        mvprintw(ScreenHeight / 2 - 3, ScreenWidth / 2 + 27, "Invalid Email!");
        werase(EmailWindow);
        refresh();
        box(EmailWindow, 0, 0);
        mvwgetnstr(EmailWindow, 1, 1, Email, 50);
    }

    mvprintw(ScreenHeight / 2 - 3, ScreenWidth / 2 + 27, "               ");
    refresh();
    mvwgetnstr(UsernameWindow, 1, 1, Username, 32);
    while (!ValidateUsername(Username))
    {
        mvprintw(ScreenHeight / 2 + 3, ScreenWidth / 2 + 20, "Username is already taken!");
        werase(UsernameWindow);
        refresh();
        box(UsernameWindow, 0, 0);
        mvwgetnstr(UsernameWindow, 1, 1, Username, 32);
    }
    mvprintw(ScreenHeight / 2 + 3, ScreenWidth / 2 + 20, "                          ");
    refresh();

    ;

    mvwgetnstr(PasswordWindow, 1, 1, Password, 33);
    while (!ValidatePassword(Password))
    {
        mvprintw(ScreenHeight / 2 + 9, ScreenWidth / 2 + 20, "Invalid Password!");
        refresh();
        werase(PasswordWindow);
        box(PasswordWindow, 0, 0);
        mvwgetnstr(PasswordWindow, 1, 1, Password, 33);
    }
    mvprintw(ScreenHeight / 2 + 9, ScreenWidth / 2 + 20, "                              ");
    refresh();

    char parent_dir[50] = "user data", UserFolder[50];
    strcpy(UserFolder, Username);
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", parent_dir, UserFolder);

    if (mkdir(dir_path, 0777) == 0)
    {
        printf("Directory '%s' created successfully inside '%s'.\n", UserFolder, parent_dir);
    }
    else
    {
        perror("Error creating directory");
    }

    char userInfoFileName[100];
    sprintf(userInfoFileName, "user data/%s/%s.txt", Username, Username);
    FILE *UserInfo = fopen(userInfoFileName, "a");
    fprintf(UserInfo, "Username: %s\nEmail: %s\nPassword: %s\n", Username, Email, Password);
    fclose(UserInfo);
    FILE *UsernameFile = fopen("user data/usernames.txt", "a");
    fprintf(UsernameFile, "%s\n", Username);
    fflush(UsernameFile);
    fclose(UsernameFile);
    strcpy(User, Username);
    logged = 1;
    char ScoreFileName[128];
    snprintf(ScoreFileName, sizeof(ScoreFileName), "user data/%s/score.txt", User);
    FILE *file = fopen(ScoreFileName, "w");
    fprintf(file, "0\n0\n0\n");
    fclose(file);
    return;
}

char *ReadFileLine(int line, int size, FILE *file)
{
    char *text = (char *)malloc(size * sizeof(char));
    int CurrentLine = 1;
    while (fgets(text, size, file))
    {
        if (CurrentLine++ == line)
        {
            return text;
        }
    }
}

void LoginPage()
{
    clear();
    echo();
    curs_set(1);
    mvprintw(ScreenHeight / 2 - 6, ScreenWidth / 2 - 4, "Username");
    mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 4, "Password");
    refresh();

    WINDOW *UsernameWindow = newwin(3, 32, (ScreenHeight / 2) - 4, ScreenWidth / 2 - 15);
    box(UsernameWindow, 0, 0);
    WINDOW *PasswordWindow = newwin(3, 32, (ScreenHeight / 2) + 2, ScreenWidth / 2 - 15);
    box(PasswordWindow, 0, 0);

    refresh();
    wrefresh(UsernameWindow);
    wrefresh(PasswordWindow);
    mvprintw(0, 0, "Press \"shift + q\" to go back. if you want to continue press any other key.");
    refresh();
    char c = getch();
    if (c == 'Q')
    {
        SignOrLog();
        return;
    }
    else
    {
        mvprintw(0, 0, "                                                                                             ");
        refresh();
    }
    char Password[33], Username[33];
    mvwgetnstr(UsernameWindow, 1, 1, Username, 33);
    FILE *UsernameFile = fopen("user data/usernames.txt", "r");

    char PreviousUser[50];
    int MatchFound = 0;
    while (!MatchFound)
    {
        while (fgets(PreviousUser, 50, UsernameFile))
        {
            PreviousUser[strlen(PreviousUser) - 1] = '\0';
            if (strcmp(Username, PreviousUser) == 0)
            {
                MatchFound = 1;
                break;
            }
        }
        rewind(UsernameFile);
        if (MatchFound)
        {
            break;
        }
        mvprintw(ScreenHeight / 2 - 3, ScreenWidth / 2 + 20, "Username not found!");
        werase(UsernameWindow);
        box(UsernameWindow, 0, 0);
        wrefresh(UsernameWindow);
        refresh();
        mvwgetnstr(UsernameWindow, 1, 1, Username, 33);
    }
    mvprintw(ScreenHeight / 2 - 3, ScreenWidth / 2 + 20, "                    ");
    refresh();

    char UserDataFilename[80];
    sprintf(UserDataFilename, "user data/%s/%s.txt", Username, Username);
    FILE *UserData = fopen(UserDataFilename, "r");

    char *UserPassword = ReadFileLine(3, 50, UserData);
    if (UserPassword[strlen(UserPassword) - 1] == '\n')
    {
        UserPassword[strlen(UserPassword) - 1] = '\0';
    }

    mvwgetnstr(PasswordWindow, 1, 1, Password, 33);
    while (strcmp(UserPassword + 10, Password) != 0)
    {
        mvprintw(ScreenHeight / 2 + 3, ScreenWidth / 2 + 20, "Wrong password!");
        werase(PasswordWindow);
        box(PasswordWindow, 0, 0);
        wrefresh(PasswordWindow);
        refresh();
        mvwgetnstr(PasswordWindow, 1, 1, Password, 33);
    }

    fclose(UserData);
    fclose(UsernameFile);
    strcpy(User, Username);
    logged = 1;
    return;
}

Button *MakeButton(int y, int x, const char *name)
{
    Button *button = (Button *)malloc(sizeof(Button));
    strcpy(button->name, name);
    button->y = y;
    button->x = x;
    return button;
}

WINDOW *MakeWindow(Button *button)
{
    WINDOW *win = newwin(3, 15, button->y, button->x);
    return win;
}

int MainMenuSetup(int res)
{
    clear();
    initscr();
    noecho();
    raw();
    cbreak();
    curs_set(0);
    start_color();
    keypad(stdscr, TRUE);
    const char MainMenuButtonNames[5][20] = {"NEW GAME", "RESUME", "SETTINGS", "SCOREBOARD", "EXIT"};
    Button *MainMenuButton[5];
    WINDOW *MainMenuWindow[5];

    for (int i = 0; i < 5; i++)
    {
        MainMenuButton[i] = MakeButton(15 + 4 * i, 90, MainMenuButtonNames[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        MainMenuWindow[i] = MakeWindow(MainMenuButton[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        box(MainMenuWindow[i], 0, 0);
        mvwprintw(MainMenuWindow[i], 1, 2, "%s", MainMenuButton[i]->name);
    }
    refresh();
    for (int i = 0; i < 5; i++)
    {
        wrefresh(MainMenuWindow[i]);
    }
    init_pair(BLACK, COLOR_BLACK, COLOR_WHITE);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    int Choice = 0;
    int PrevChoice = 0;
    while (1)
    {
        wbkgd(MainMenuWindow[PrevChoice], COLOR_PAIR(WHITE));
        wbkgd(MainMenuWindow[Choice], COLOR_PAIR(BLACK));
        refresh();
        wrefresh(MainMenuWindow[PrevChoice]);
        wrefresh(MainMenuWindow[Choice]);
        PrevChoice = Choice;
        int c = getch();
        if (c == KEY_UP || c == KEY_RIGHT)
        {
            if (Choice > 0)
            {
                Choice--;
            }
        }
        else if (c == KEY_DOWN || c == KEY_LEFT)
        {
            if (Choice < 4)
            {
                Choice++;
            }
        }
        if (c == '\n')
        {
            switch (Choice)
            {
            case 0:
                NewGame();
                return 1;
                break;
            case 1:
                if (res == 1)
                {
                    clear();
                    for (int i = 0; i < 5; i++)
                    {
                        delwin(MainMenuWindow[i]);
                        free(MainMenuButton[i]);
                    }
                    return 0;
                }
                mvprintw(2, 2, "you haven't started a new game.");
                refresh();
                break;
            case 2:
                Settings();
                clear();
                for (int i = 0; i < 5; i++)
                {
                    box(MainMenuWindow[i], 0, 0);
                    mvwprintw(MainMenuWindow[i], 1, 2, "%s", MainMenuButton[i]->name);
                    refresh();
                    wrefresh(MainMenuWindow[i]);
                }
                refresh();
                break;
            case 3:
                ScoreBoard();
                clear();
                for (int i = 0; i < 5; i++)
                {
                    box(MainMenuWindow[i], 0, 0);
                    mvwprintw(MainMenuWindow[i], 1, 2, "%s", MainMenuButton[i]->name);
                    refresh();
                    wrefresh(MainMenuWindow[i]);
                }
                refresh();
                break;
            case 4:
                Exit();
                return 5;
                break;
            }
        }
    }
}

void Exit()
{
    endwin();
    exit(0);
}

void Settings()
{
    clear();
    initscr();
    WINDOW *DifficultyWin = newwin(3, 10, ScreenHeight / 2 - 5, ScreenWidth / 2 - 3);
    WINDOW *ColorWin = newwin(3, 10, ScreenHeight / 2, ScreenWidth / 2 - 3);
    WINDOW *MusicWin = newwin(3, 10, ScreenHeight / 2 + 5, ScreenWidth / 2 - 3);
    int prevmusic = 1;
    init_pair(BLACK, COLOR_BLACK, COLOR_WHITE);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    mvwprintw(DifficultyWin, 1, 0, "DIFFICULTY");
    mvwprintw(ColorWin, 1, 0, "HERO COLOR");
    mvwprintw(MusicWin, 1, 0, "Music");
    mvprintw(0, 0, "Press \"shift + q\" to go back to main menu.");
    refresh();
    wrefresh(ColorWin);
    wrefresh(DifficultyWin);
    wrefresh(MusicWin);
    int Choice = 1, c;
    refresh();
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music *music1 = Mix_LoadMUS("music1.mp3");
    Mix_Music *music2 = Mix_LoadMUS("music2.mp3");
    while (1)
    {
        if (c == 'Q')
        {
            return;
        }
        if ((c == KEY_DOWN) && (Choice < 3))
        {
            Choice++;
        }
        else if ((c == KEY_UP) && (Choice > 1))
        {
            Choice--;
        }
        else if (c == KEY_RIGHT)
        {
            if ((Choice == 1) && (Difficulty == 2))
            {
                Difficulty = 1;
            }
            else if ((Choice == 2) && (HeroColor < 3))
            {
                HeroColor++;
            }
            else if ((Choice == 3) && (Music < 2))
            {
                Music++;
            }
        }
        else if (c == KEY_LEFT)
        {
            if ((Choice == 1) && (Difficulty == 1))
            {
                Difficulty = 2;
            }
            else if ((Choice == 2) && (HeroColor > 1))
            {
                HeroColor--;
            }
            else if ((Choice == 3) && (Music > 0))
            {
                Music--;
            }
        }
        refresh();
        if (Difficulty == 1)
        {
            mvprintw(ScreenHeight / 2 - 4, ScreenWidth / 2 + 10, "Easy");
            Scarcity = 1;
        }
        else if (Difficulty == 2)
        {
            Scarcity = 2;
            mvprintw(ScreenHeight / 2 - 4, ScreenWidth / 2 + 10, "Hard  ");
        }
        switch (HeroColor)
        {
        case 1:
            mvprintw(ScreenHeight / 2 + 1, ScreenWidth / 2 + 10, "YELLOW");
            break;
        case 2:
            mvprintw(ScreenHeight / 2 + 1, ScreenWidth / 2 + 10, "RED   ");
            break;
        case 3:
            mvprintw(ScreenHeight / 2 + 1, ScreenWidth / 2 + 10, "BLUE  ");
            break;
        default:
            break;
        }
        if (Music == 1)
        {
            mvprintw(ScreenHeight / 2 + 6, ScreenWidth / 2 + 10, "1  ");
        }
        else if (Music == 2)
        {
            mvprintw(ScreenHeight / 2 + 6, ScreenWidth / 2 + 10, "2  ");
        }
        else if (Music == 0)
        {
            mvprintw(ScreenHeight / 2 + 6, ScreenWidth / 2 + 10, "off");   
        }
        if (Choice == 1)
        { // color
            wbkgd(DifficultyWin, COLOR_PAIR(BLACK));
            wbkgd(ColorWin, COLOR_PAIR(WHITE));
            wbkgd(MusicWin, COLOR_PAIR(WHITE));
        }
        else if (Choice == 2)
        { // difficulty
            wbkgd(ColorWin, COLOR_PAIR(BLACK));
            wbkgd(MusicWin, COLOR_PAIR(WHITE));
            wbkgd(DifficultyWin, COLOR_PAIR(WHITE));
        }
        else if (Choice == 3)
        { // Music
            wbkgd(MusicWin, COLOR_PAIR(BLACK));
            wbkgd(ColorWin, COLOR_PAIR(WHITE));
            wbkgd(DifficultyWin, COLOR_PAIR(WHITE));
        }
        if (Music == 0)
        {
            Mix_HaltMusic();
            prevmusic = Music;
        }
        if (Music == 1 && prevmusic != Music)
        {
            Mix_PlayMusic(music1, -1);
            prevmusic = Music;
        }
        else if (Music == 2 && prevmusic != Music)
        {
            Mix_PlayMusic(music2, -1);
            prevmusic = Music;
        }
        refresh();
        wrefresh(ColorWin);
        wrefresh(DifficultyWin);
        wrefresh(MusicWin);
        refresh();
        c = getch();
    }
}

void NewGame()
{
    Level = 1;
    clear();
    srand(time(NULL));
    initscr();
    int RoomCount = 6 + (rand() % 4);
    Room **rooms = (Room **)malloc(RoomCount * sizeof(Room *));
    char **map = (char **)malloc(ScreenHeight * sizeof(char *));
    for (int i = 0; i < ScreenHeight; i++)
    {
        (map)[i] = (char *)malloc(ScreenWidth * sizeof(char));
        for (int j = 0; j < ScreenWidth; j++)
        {
            (map)[i][j] = ' ';
        }
    }

    MapInit(&map, rooms, RoomCount, NULL);
    Player *player = (Player *)malloc(sizeof(Player));
    InitPlayer(player);
    PlayGame(player, map, rooms, RoomCount);
    return;
}

void InitPlayer(Player *player)
{
    player->PP = '.';
    player->Gold = 0;
    player->xp = 0;
    player->HP = 20;
    player->FullHP = 20;
    player->Energy = 20;
    player->FullEnergy = 20;
    player->score = 0;
    player->wielded = MACE;
    player->treasure = 0;
    for (int i = 0; i < 4; i++)
    {
        player->weapninventory[i] = 0;
    }
    player->weapninventory[MACE] = 1;

    for (int i = 0; i < 4; i++)
    {
        player->foodinventory[i] = 0;
    }
    player->foodinventory[NORMALFOOD] = 1;
    return;
}

void PlayGame(Player *player, char **map1, Room **rooms1, int RoomCount1)
{
    // colors
    init_color(COLOR_YELLOW, 1000, 800, 0);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_color(9, 1000, 500, 0);    // ORANGE
    init_color(10, 1000, 300, 600); // PINK
    init_color(11, 1000, 0, 1000);  // PURPLE
    init_color(12, 255, 223, 127);  // BROWN
    init_pair(6, 9, COLOR_BLACK);
    init_pair(8, 10, COLOR_BLACK);
    init_pair(9, 11, COLOR_BLACK);
    init_pair(10, 12, COLOR_BLACK);
    int LevelChange, LevelInit = 1, MoveCount = 0, ShowMap = 0;

    int strengthTime = 0, SpeedTime = 0, MaxLevel = 1;

    char ***map = (char ***)malloc(4 * sizeof(char **)); // all levels maps
    map[0] = map1;

    Room ***rooms = (Room ***)malloc(4 * sizeof(Room **)); // all levels rooms
    rooms[0] = rooms1;

    int *RoomCount = (int *)malloc(4 * sizeof(int)); // all levels roomcounts
    RoomCount[0] = RoomCount1;

    Monster ***monster = (Monster ***)malloc(4 * sizeof(Monster **));
    int MonsterCount[4] = {0};
    for (int i = 0; i < 4; i++)
    {
        monster[i] = (Monster **)malloc(100 * sizeof(Monster *));
        for (int j = 0; j < 100; j++)
        {
            monster[i][j] = (Monster *)malloc(sizeof(Monster));
        }
    }
    char ***VisibleMap = (char ***)malloc(4 * sizeof(char **));
    int SpawnRoom = SpawnPlayer(rooms1, RoomCount1, player, map1);

    int NormalFoodAge[50] = {0};
    int MagicFoodAge[50] = {0};
    int FineFoodAge[50] = {0};

    int MonsterAggroAge[4][100];

    for (int i = 1; i < 4; i++)
    {
        map[i] = (char **)malloc(sizeof(char *) * ScreenHeight);
        for (int j = 0; j < ScreenHeight; j++)
        {
            map[i][j] = (char *)malloc(sizeof(char) * ScreenWidth);
        }
    }
    for (int x = 0; x < 4; x++)
    {
        VisibleMap[x] = (char **)malloc(ScreenHeight * sizeof(char *));
        for (int i = 0; i < ScreenHeight; i++)
        {
            (VisibleMap)[x][i] = (char *)malloc(ScreenWidth * sizeof(char));
            for (int j = 0; j < ScreenWidth; j++)
            {
                (VisibleMap)[x][i][j] = ' ';
            }
        }
    }

    int StairRoom = SpawnStairs(map1, rooms1, RoomCount1, SpawnRoom);

    InitMonsterLevel(map[Level - 1], monster[Level - 1], MonsterCount);

    while (1)
    {
        for (int i = 0; i < RoomCount[Level - 1]; i++)
        {
            if (rooms[Level - 1][i]->show == 1)
            {
                for (int y = rooms[Level - 1][i]->y1; y <= rooms[Level - 1][i]->y2; y++)
                {
                    for (int x = rooms[Level - 1][i]->x1; x <= rooms[Level - 1][i]->x2; x++)
                    {
                        VisibleMap[Level - 1][y][x] = map[Level - 1][y][x];
                    }
                }
            }
        }
        for (int i = UpLimit; i <= DownLimit; i++)
        {
            for (int j = LeftLimit; j <= RightLimit; j++)
            {
                if (!ShowMap)
                {
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                }
                else
                {
                    mvprintw(i, j, "%c", map[Level - 1][i][j]);
                }
            }
        }

        for (int i = 0; i < RoomCount[Level - 1]; i++)
        {
            if (rooms[Level - 1][i]->type == 2)
            {
                for (int y = rooms[Level - 1][i]->y1; y <= rooms[Level - 1][i]->y2; y++)
                {
                    for (int x = rooms[Level - 1][i]->x1; x <= rooms[Level - 1][i]->x2; x++)
                    {

                        attron(COLOR_PAIR(YELLOW));
                        if (!ShowMap)
                        {
                            mvprintw(y, x, "%c", VisibleMap[Level - 1][y][x]);
                        }
                        else
                        {
                            mvprintw(y, x, "%c", map[Level - 1][y][x]);
                        }
                        attroff(COLOR_PAIR(YELLOW));
                    }
                }
            }
            else if (rooms[Level - 1][i]->type == 1)
            {
                for (int y = rooms[Level - 1][i]->y1; y <= rooms[Level - 1][i]->y2; y++)
                {
                    for (int x = rooms[Level - 1][i]->x1; x <= rooms[Level - 1][i]->x2; x++)
                    {
                        attron(COLOR_PAIR(MAGENTA));
                        if (!ShowMap)
                        {
                            mvprintw(y, x, "%c", VisibleMap[Level - 1][y][x]);
                        }
                        else
                        {
                            mvprintw(y, x, "%c", map[Level - 1][y][x]);
                        }
                        attroff(COLOR_PAIR(MAGENTA));
                    }
                }
            }
        }

        for (int i = UpLimit; i <= DownLimit; i++)
        {
            for (int j = LeftLimit; j <= RightLimit; j++)
            {
                if (map[Level - 1][i][j] == PlayerSymbol)
                {
                    attron(COLOR_PAIR(HeroColor));
                    mvprintw(i, j, "%c", map[Level - 1][i][j]);
                    attroff(COLOR_PAIR(HeroColor));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '=')
                {
                    attron(COLOR_PAIR(PURPLE));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(PURPLE));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '*')
                {
                    attron(COLOR_PAIR(YELLOW));
                    mvprintw(i, j, "%c", VisibleMap[Level - YELLOW][i][j]);
                    attroff(COLOR_PAIR(1));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '-' || VisibleMap[Level - 1][i][j] == '|')
                {
                    attron(COLOR_PAIR(ORANGE));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(ORANGE));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == FOOD_SYMBOL)
                {
                    attron(COLOR_PAIR(GREEN));
                    mvprintw(i, j, "H");
                    attroff(COLOR_PAIR(GREEN));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '3')
                {
                    attron(COLOR_PAIR(Cyan));
                    mvaddstr(i, j, "\u2666");
                    attroff(COLOR_PAIR(Cyan));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == SPELL_SYMBOL)
                {
                    attron(COLOR_PAIR(Cyan));
                    mvaddstr(i, j, "\u2697");
                    attroff(COLOR_PAIR(Cyan));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '#')
                {
                    attron(COLOR_PAIR(MAGENTA));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(MAGENTA));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == MACE)
                {
                    mvaddstr(i, j, "\u2692");
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == DAGGER)
                {
                    attron(COLOR_PAIR(MAGENTA));
                    mvprintw(i, j, "d");
                    attroff(COLOR_PAIR(MAGENTA));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == MAGICWAND || VisibleMap[Level - 1][i][j] == DROPPEDWAND)
                {
                    attron(COLOR_PAIR(MAGENTA));
                    mvprintw(i, j, "m");
                    attroff(COLOR_PAIR(MAGENTA));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == ARROW || VisibleMap[Level - 1][i][j] == DROPPEDARROW)
                {
                    attron(COLOR_PAIR(MAGENTA));
                    mvaddstr(i, j, "\u27B3");
                    attroff(COLOR_PAIR(MAGENTA));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == SWORD)
                {
                    attron(COLOR_PAIR(MAGENTA));
                    mvaddstr(i, j, "\u2694");
                    attroff(COLOR_PAIR(MAGENTA));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == '+' || VisibleMap[Level - 1][i][j] == 'O')
                {
                    attron(COLOR_PAIR(PINK));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(PINK));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == DEAMON)
                {
                    attron(COLOR_PAIR(RED));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(RED));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == FIRE_BREATHING_MONSTER)
                {
                    attron(COLOR_PAIR(ORANGE));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(ORANGE));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == GIANT)
                {
                    attron(COLOR_PAIR(BROWN));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(BROWN));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == SNAKE)
                {
                    attron(COLOR_PAIR(YELLOW));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(YELLOW));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == UNDEAD)
                {
                    attron(COLOR_PAIR(GREEN));
                    mvprintw(i, j, "%c", VisibleMap[Level - 1][i][j]);
                    attroff(COLOR_PAIR(GREEN));
                    continue;
                }
                else if (VisibleMap[Level - 1][i][j] == TREASURE)
                {
                    mvprintw(i, j, "\u2696");
                }
            }

            refresh();
        }

        mvprintw(1, 100, "Move: %d", MoveCount);
        attron(COLOR_PAIR(1));
        mvprintw(ScreenHeight - 2, 2, "Gold: %d RC", player->Gold);
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        mvprintw(ScreenHeight - 2, 22, "HP: %d / %d  ", player->HP, player->FullHP);
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(4));
        mvprintw(ScreenHeight - 2, 41, "Energy: %d / %d ", player->Energy, player->FullEnergy);
        attroff(COLOR_PAIR(4));
        mvprintw(ScreenHeight - 2, 62, "Score: %d", player->score);
        mvprintw(ScreenHeight - 2, 77, "Floor: %d", Level);

        refresh();

        LevelChange = Movement(map[Level - 1], VisibleMap[Level - 1], player, rooms[Level - 1], RoomCount[Level - 1], &ShowMap, NormalFoodAge, MagicFoodAge, FineFoodAge, -1);
        if (LevelChange == 1)
        {
            if (Level == LevelInit)
            {
                Level++;
                MaxLevel++;
                RoomCount[Level - 1] = 6 + (rand() % 4);
                rooms[Level - 1] = (Room **)malloc(RoomCount[Level - 1] * sizeof(Room **));
                MapInit(&map[Level - 1], rooms[Level - 1], RoomCount[Level - 1], rooms[Level - 2][StairRoom]);
                if (Level != 4)
                {
                    StairRoom = SpawnStairs(map[Level - 1], rooms[Level - 1], RoomCount[Level - 1], 0);
                }
                map[Level - 1][player->y][player->x] = PlayerSymbol;
                LevelInit++;
                InitMonsterLevel(map[Level - 1], monster[Level - 1], MonsterCount);
            }
            else
            {
                Level++;
            }
        }
        else if (LevelChange == -1)
        {
            int gstate = MainMenuSetup(1);
            if (gstate == 5)
            {
                break;
            }
            else if (gstate == 0)
            {
                continue;
            }
        }
        else if (LevelChange == 2)
        {
            Level--;
        }
        else if (LevelChange == 4)
        {
            LevelChange = Movement(map[Level - 1], VisibleMap[Level - 1], player, rooms[Level - 1], RoomCount[Level - 1], &ShowMap, NormalFoodAge, MagicFoodAge, FineFoodAge, 0);
        }
        else if (LevelChange == 5)
        {
            Attack(map[Level - 1], player, monster[Level - 1], MonsterCount[Level - 1]);
        }
        else if (LevelChange == 6)
        {
            // Project(map[Level - 1], player, monster[Level - 1], MonsterCount[Level - 1], rooms[Level - 1]);
        }
        refresh();
        if (LevelChange != 3)
        {
            MonsterMovement(MonsterAggroAge, MonsterCount[Level - 1], monster[Level - 1], player, map[Level - 1]);
            MoveCount++;
            if (speed == 2)
            {
                mvprintw(3, 2, "I'm fast af boi!");
                SpeedTime++;
                if (SpeedTime == 10)
                {
                    SpeedTime = 0;
                    speed = 1;
                    mvprintw(3, 2, "                ");
                }
                for (int i = 0; i < player->foodinventory[MAGICFOOD]; i++)
                {
                    MagicFoodAge[i]++;
                    if (MagicFoodAge[i] > EXPIRATION)
                    {
                        DeleteElement(50, MagicFoodAge, i);
                        (player->foodinventory[MAGICFOOD])--;
                        (player->foodinventory[SPOILEDFOOD])++;
                        i--;
                    }
                }
            }
            if (strength == 2)
            {
                mvprintw(4, 2, "I'm strong af");
                strengthTime++;
                if (strengthTime == 10)
                {
                    mvprintw(4, 2, "                ");
                    strengthTime = 0;
                    strength = 1;
                }
            }

            // normal food
            for (int i = 0; i < player->foodinventory[NORMALFOOD]; i++)
            {
                NormalFoodAge[i]++;
                if (NormalFoodAge[i] > EXPIRATION)
                {
                    DeleteElement(50, NormalFoodAge, i);
                    (player->foodinventory[NORMALFOOD])--;
                    (player->foodinventory[SPOILEDFOOD])++;
                    i--;
                    continue;
                }
            }
            // fine food
            for (int i = 0; i < player->foodinventory[FINEFOOD]; i++)
            {
                FineFoodAge[i]++;
                if (FineFoodAge[i] > EXPIRATION)
                {
                    DeleteElement(50, FineFoodAge, i);
                    (player->foodinventory[FINEFOOD])--;
                    (player->foodinventory[SPOILEDFOOD])++;
                    i--;
                    continue;
                }
            }

            // magic food
            for (int i = 0; i < player->foodinventory[MAGICFOOD]; i++)
            {
                MagicFoodAge[i]++;
                if (MagicFoodAge[i] > EXPIRATION)
                {
                    DeleteElement(50, MagicFoodAge, i);
                    (player->foodinventory[MAGICFOOD])--;
                    (player->foodinventory[NORMALFOOD])++;
                    i--;
                    continue;
                }
            }
        }
        for (int i = 0; i < MonsterCount[Level - 1]; i++)
        {
            if (monster[Level - 1][i]->HP <= 0)
            {
                kill(map[Level - 1], monster[Level - 1][i]);
            }
        }
        mvprintw(player->y, player->x, "%c", PlayerSymbol);
        refresh();
        if (player->HP <= 0)
        {
            lose(player);
            break;
        }
        if ((player->treasure) == 1)
        {
            wingame(player);
            break;
        }
    }
}

void wingame(Player *player)
{
    clear();
    if (logged)
    {

        char ScoreFileName[128];
        snprintf(ScoreFileName, sizeof(ScoreFileName), "user data/%s/score.txt", User);
        FILE *file = fopen(ScoreFileName, "r+");
        int gold, score, gamesplayed;
        fseek(file, 0, SEEK_END);
        long size = ftell(file);

        if (size == 0)
        {
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", (player->Gold));
            fprintf(file, "%d\n", player->score);
            fprintf(file, "%d", 1);
            fclose(file);
        }
        else
        {
            fseek(file, 0, SEEK_SET);
            fscanf(file, "%d", &gold);
            fscanf(file, "%d", &score);
            fscanf(file, "%d", &gamesplayed);
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", gold + (player->Gold));
            fprintf(file, "%d\n", score + (player->score));
            fprintf(file, "%d", gamesplayed + 1);
            fclose(file);
        }
    }
    const char *art[] =
        {"⠀⠀⠀⠀⢀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⠀⠀⠀⠀",
         "⢠⣤⣤⣤⣼⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣄⣤⣤⣠",
         "⢸⠀⡶⠶⠾⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡷⠶⠶⡆⡼",
         "⠈⡇⢷⠀⠀⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠇⠀⢸⢁⡗",
         "⠀⢹⡘⡆⠀⢹⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡸⠀⢀⡏⡼⠀",
         "⠀⠀⢳⡙⣆⠈⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠇⢀⠞⡼⠁⠀",
         "⠀⠀⠀⠙⣌⠳⣼⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣞⡴⣫⠞⠀⠀⠀",
         "⠀⠀⠀⠀⠈⠓⢮⣻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡼⣩⠞⠉⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠉⠛⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠞⠋⠁⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠳⢤⣀⠀⠀⠀⠀⢀⣠⠖⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⡇⢸⡏⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠖⠒⠓⠚⠓⠒⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⣀⣠⣞⣉⣉⣉⣉⣉⣉⣉⣉⣉⣉⣙⣆⣀⡀⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀",
         "⠀⠀⠀⠀⠀⠀⠀⠓⠲⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶⠶⠖⠃⠀⠀⠀⠀⠀⠀",
         "       CONGRATS YOU WON!!         "};

    for (int i = 0; i < sizeof(art) / sizeof(art[0]); i++)
    {
        mvprintw(21 + i, 80, "%s", art[i]);
    }

    refresh();
    getch();
    MainMenuSetup(0);
    return;
}

void lose(Player *player)
{
    if (logged)
    {

        char ScoreFileName[128];
        snprintf(ScoreFileName,sizeof(ScoreFileName) , "user data/%s/score.txt", User);
        FILE *file = fopen(ScoreFileName, "r+");
        int gold, score, gamesplayed;
        fseek(file, 0, SEEK_END);
        long size = ftell(file);

        if (size == 0)
        {
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", (player->Gold));
            fprintf(file, "%d\n", player->score);
            fprintf(file, "%d", 1);
            fclose(file);
        }

        else
        {
            fseek(file, 0, SEEK_SET);
            fscanf(file, "%d", &gold);
            fscanf(file, "%d", &score);
            fscanf(file, "%d", &gamesplayed);
            fseek(file, 0, SEEK_SET);
            fprintf(file, "%d\n", gold + (player->Gold));
            fprintf(file, "%d\n", score + (player->score));
            fprintf(file, "%d", gamesplayed + 1);
            fclose(file);
        }
    }
    clear();
    const char *art[] = {
        "  ██╗   ██╗ ██████╗ ██╗   ██╗    ██████╗ ██╗███████╗██████╗ ",
        "  ╚██╗ ██╔╝██╔═══██╗██║   ██║    ██╔══██╗██║██╔════╝██╔══██╗",
        "   ╚████╔╝ ██║   ██║██║   ██║    ██║  ██║██║█████╗  ██║  ██║",
        "    ╚██╔╝  ██║   ██║██║   ██║    ██║  ██║██║██╔══╝  ██║  ██║",
        "     ██║   ╚██████╔╝╚██████╔╝    ██████╔╝██║███████╗██████╔╝",
        "     ╚═╝    ╚═════╝  ╚═════╝     ╚═════╝ ╚═╝╚══════╝╚═════╝ ",
        "                     ██╗      ██████╗ ██╗",
        "                     ██║     ██╔═══██╗██║",
        "                     ██║     ██║   ██║██║",
        "                     ██║     ██║   ██║██║",
        "                     ███████╗╚██████╔╝███████╗",
        "                     ╚══════╝ ╚═════╝ ╚══════╝"};

    for (int i = 0; i < sizeof(art) / sizeof(art[0]); i++)
    {
        mvprintw(21 + i, 60, "%s", art[i]);
    }

    refresh();
    getch();
    MainMenuSetup(0);
    return;
}

void kill(char **map, Monster *monster)
{
    map[monster->y][monster->x] = monster->pp;
    monster->aggro = -99999;
    monster->damage = 0;
    monster->name = '.';
    monster->dead = 0;
    monster->y = 0;
    monster->x = 0;
    return;
}

void InitMonsterLevel(char **map, Monster **monster, int *MonsterCount)
{
    for (int i = 0; i < ScreenHeight; i++)
    {
        for (int j = 0; j < ScreenWidth; j++)
        {
            if (map[i][j] == DEAMON)
            {
                attron(COLOR_PAIR(RED));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(RED));
                monster[(MonsterCount[Level - 1])++] = InitMonster(map, i, j);
                continue;
            }
            else if (map[i][j] == FIRE_BREATHING_MONSTER)
            {
                attron(COLOR_PAIR(ORANGE));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(ORANGE));
                monster[(MonsterCount[Level - 1])++] = InitMonster(map, i, j);
                continue;
            }
            else if (map[i][j] == GIANT)
            {
                attron(COLOR_PAIR(BROWN));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(BROWN));
                monster[(MonsterCount[Level - 1])++] = InitMonster(map, i, j);
                continue;
            }
            else if (map[i][j] == SNAKE)
            {
                attron(COLOR_PAIR(YELLOW));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(YELLOW));
                monster[(MonsterCount[Level - 1])++] = InitMonster(map, i, j);
                continue;
            }
            else if (map[i][j] == UNDEAD)
            {
                attron(COLOR_PAIR(GREEN));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(GREEN));
                monster[(MonsterCount[Level - 1])++] = InitMonster(map, i, j);
                continue;
            }
        }
    }
}

void Attack(char **map, Player *player, Monster **monster, int MonsterCount)
{
    for (int i = 0; i < MonsterCount; i++)
    {
        if (dif(monster[i]->x - player->x) && dif(monster[i]->y - player->y))
        {
            switch (player->wielded)
            {
            case MACE:
                monster[i]->HP -= 5 * strength;
                player->score++;
                break;
            case SWORD:
                monster[i]->HP -= 10 * strength;
                player->score++;
            default:
                break;
            }
        }
    }
    return;
}

Monster *InitMonster(char **map, int y, int x)
{
    Monster *monster = (Monster *)malloc(sizeof(Monster));
    monster->aggro = 0;
    monster->x = x;
    monster->y = y;
    monster->dead = 0;
    monster->pp = '.';
    switch (map[y][x])
    {
    case DEAMON:
        monster->type = 1;
        monster->damage = 1 * Difficulty;
        monster->HP = 5 + ((rand() % 5) * Difficulty);
        monster->name = 'D';
        break;
    case FIRE_BREATHING_MONSTER:
        monster->type = 2;
        monster->damage = 3 * Difficulty;
        monster->HP = 10 + ((rand() % 5) * Difficulty);
        monster->name = 'F';
        break;
    case GIANT:
        monster->type = 3;
        monster->damage = 2 * Difficulty;
        monster->HP = 10 + ((rand() % 5) * Difficulty);
        monster->name = 'G';
        break;
    case SNAKE:
        monster->type = 4;
        monster->damage = 3 * Difficulty;
        monster->HP = 20 + ((rand() % 5) * Difficulty);
        monster->name = 'S';
        break;
    case UNDEAD:
        monster->type = 5;
        monster->damage = 3 * Difficulty;
        monster->HP = 30 + ((rand() % 5) * Difficulty);
        monster->name = 'U';
        break;
    default:
        break;
    }
    return monster;
}

int sign(int x)
{
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}

int dif(int a)
{
    if (a >= -1 && a <= 1)
        return 1;
    return 0;
}

int CheckCoMonster(char **map, int y, int x)
{
    switch (map[y][x])
    {
    case '@':
        return 2;
        break;
    case 'O':
        return 4;
        break;
    case '-':
    case '|':
    case '+':
    case '%':
    case '=':
    case DEAMON:
    case UNDEAD:
    case GIANT:
    case SNAKE:
    case FIRE_BREATHING_MONSTER:
        return 0;
        break;
    case MAGICWAND:
    case DAGGER:
    case SWORD:
    case ARROW:
    case MACE:
    case '*':
    case '3':
    case FOOD_SYMBOL:
    case '.':
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

void MonsterMovement(int MonsterAggroAge[4][100], int MonsterCount, Monster **monster, Player *player, char **map)
{
    int mx, my, px, py, mpp;
    for (int i = 0; i < MonsterCount; i++)
    {
        if (monster[i]->dead == 1)
        {
            continue;
        }
        mx = monster[i]->x;
        my = monster[i]->y;
        px = player->x;
        py = player->y;
        mpp = monster[i]->pp;
        mvprintw(0, 3 * i, "%c%d%d", monster[i]->name, my, mx);
        int next = CheckCoMonster(map, my + sign(py - my), mx + sign(px - mx));
        if (next == 2)
        {
            player->HP -= monster[i]->damage;
        }
        else if (next == 4)
        {
            if (CheckCoMonster(map, my + sign(py - my), mx == 1))
            {
                map[my][mx] = mpp;
                my += sign(py - my);
                monster[i]->pp = map[my][mx];
                map[my][mx] = monster[i]->name;
                monster[i]->y = my;
            }
            else if (CheckCoMonster(map, my, mx + sign(px - mx)) == 1)
            {
                map[my][mx] = mpp;
                mx += sign(px - mx);
                monster[i]->pp = map[my][mx];
                map[my][mx] = monster[i]->name;
                monster[i]->x = mx;
            }
        }
        else if ((monster[i]->aggro > 0) && (next == 1))
        {
            map[my][mx] = mpp;
            my += sign(py - my);
            mx += sign(px - mx);
            monster[i]->pp = map[my][mx];
            map[my][mx] = monster[i]->name;
            monster[i]->x = mx;
            monster[i]->y = my;
        }
        if (dif(my - py) && dif(mx - px))
        {
            mvprintw(0, 10, "yeah");
            monster[i]->aggro = 5;
            if (monster[i]->type == 4)
            {
                monster[i]->aggro = 1000;
            }
        }
    }
    return;
}
int Movement(char **map, char **VisibleMap, Player *player, Room **rooms, int RoomCount, int *ShowMap, int NormalFoodAge[50], int MagicFoodAge[50], int FineFoodAge[50], int input)
{
    int c, flag = 0, x = player->x, y = player->y, pp = player->PP;
    static int MoveCount = 0;
    static int prev = 0;
    while (!flag)
    {
        if (input == 0)
        {
            c = prev;
        }
        else
        {
            c = getch();
            prev = c;
        }
        switch (c)
        {
        case KEY_UP:
            if (CheckCo(player, map, y - 1, x) == 1)
            {
                map[y--][x] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y - 1, x) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_DOWN:
            if (CheckCo(player, map, y + 1, x) == 1)
            {
                map[y++][x] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y + 1, x) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_RIGHT:
            if (CheckCo(player, map, y, x + 1) == 1)
            {
                map[y][x++] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y, x + 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_LEFT:
            if (CheckCo(player, map, y, x - 1) == 1)
            {
                map[y][x--] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y, x - 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_PPAGE:
            if (CheckCo(player, map, y - 1, x + 1) == 1)
            {
                map[y--][x++] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y - 1, x + 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_NPAGE:
            if (CheckCo(player, map, y + 1, x + 1) == 1)
            {
                map[y++][x++] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y + 1, x + 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_HOME:
            if (CheckCo(player, map, y - 1, x - 1) == 1)
            {
                map[y--][x--] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y - 1, x - 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case KEY_END:
            if (CheckCo(player, map, y + 1, x - 1) == 1)
            {
                map[y++][x--] = pp;
                flag++;
            }
            else if (CheckCo(player, map, y + 1, x - 1) == 0 && input == 0)
            {
                prev = 0;
                return 0;
            }
            break;
        case '.':
            if (player->PP == '>')
            {
                player->PP = '<';
                return 1;
            }
            break;
        case ',':
            if (player->PP == '<')
            {
                player->PP = '>';
                return 2;
            }
            break;
        case 'm':
            if ((*ShowMap) == 1)
            {
                (*ShowMap) = 0;
            }
            else if ((*ShowMap) == 0)
            {
                (*ShowMap) = 1;
            }
            return 3;
        case 'Q':
            clear();
            mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 7, "You a quitter?");
            refresh();
            int a = getch();
            if (a == '\n')
            {
                return -1;
            }
            else
            {
                return 3;
                break;
            }
        case 'e':
            LoadFoodInventory(player, NormalFoodAge, MagicFoodAge, FineFoodAge);
            return 3;
            break;
        case 'i':
            LoadWeaponInventory(player);
            return 3;
            break;
        case 's':
            LoadSpellInventory(player);
            return 3;
            break;
        case KEY_IC:
            switch (player->wielded)
            {
            case MACE:
            case SWORD:
                return 5;
                break;
            default:
                break;
            }
            break;
        case ' ':
            switch (player->wielded)
            {
            case DAGGER:
            case MAGICWAND:
            case ARROW:
                return 6;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    if (flag)
    {
        player->PP = map[y][x];
        if (map[y][x] != '#' && map[y][x] != '+' && map[y][x] != '>' && map[y][x] != '<')
        {
            player->PP = '.';
        }
        else if (map[y][x] == '+')
        {
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    if (map[y + i][x + j] == '#')
                    {
                        VisibleMap[y + i][x + j] = map[y + i][x + j];
                    }
                }
            }
            CheckDoor(rooms, map, RoomCount, y, x);
        }
        else if (map[y][x] == '#' || map[y][x] == '+')
        {
            for (int i = 0; (i <= 5) && (map[y + i][x] == '#' || map[y + i][x] == '+'); i++)
            {
                VisibleMap[y + i][x] = map[y + i][x];
            }
            for (int i = 0; (i <= 5) && (map[y][x + i] == '#' || map[y][x + i] == '+'); i++)
            {
                VisibleMap[y][x + i] = map[y][x + i];
            }
            for (int i = 0; (i >= -5) && (map[y + i][x] == '#' || map[y + i][x] == '+'); i--)
            {
                VisibleMap[y + i][x] = map[y + i][x];
            }
            for (int i = 0; (i >= -5) && (map[y][x + i] == '#' || map[y][x + i] == '+'); i--)
            {
                VisibleMap[y][x + i] = map[y][x + i];
            }
        }
        map[y][x] = PlayerSymbol;
        player->x = x;
        player->y = y;
    }
    MoveCount += flag;
    if (!(MoveCount % 40))
    {
        if (player->Energy)
        {
            (player->Energy) -= 1;
            if (player->HP < player->FullHP)
            {
                (player->HP)++;
            }
        }
        else
        {
            (player->HP)--;
        }
    }
    if (speed == 2 && input == -1)
    {
        return 4;
    }

    return 0;
}

void LoadFoodInventory(Player *player, int NormalFoodAge[50], int MagicFoodAge[50], int FineFoodAge[50])
{
    clear();
    int b, state = 1, spoilchance;
    while (state)
    {
        mvprintw(ScreenHeight / 2 - 2, ScreenWidth / 2 - 6, "1- NormalFood: %d", ((player->foodinventory[NORMALFOOD]) + (player->foodinventory[SPOILEDFOOD])));
        mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 6, "2- FineFood: %d", player->foodinventory[FINEFOOD]);
        mvprintw(ScreenHeight / 2 + 2, ScreenWidth / 2 - 6, "3- MagicFood: %d", player->foodinventory[MAGICFOOD]);
        if (player->foodinventory[NORMALFOOD] + player->foodinventory[SPOILEDFOOD])
        {
            spoilchance = rand() % (player->foodinventory[NORMALFOOD] + player->foodinventory[SPOILEDFOOD]);
        }
        else
        {
            spoilchance = 2;
        }
        b = getch();
        switch (b)
        {
        case '1':
            if (player->foodinventory[NORMALFOOD] + player->foodinventory[SPOILEDFOOD] == 0)
            {
                break;
            }
            refresh();
            if (player->foodinventory[SPOILEDFOOD] == 0)
            {
                for (int i = 0; i < 50; i++)
                {
                    if (NormalFoodAge[i] <= 100)
                    {
                        DeleteElement(50, NormalFoodAge, i);
                        break;
                    }
                }
                player->HP += 5;
                player->foodinventory[NORMALFOOD]--;
                player->Energy = player->FullEnergy;
            }
            else if (spoilchance < player->foodinventory[SPOILEDFOOD])
            {
                player->HP -= 3;
                player->foodinventory[SPOILEDFOOD]--;
                for (int i = 0; i < 50; i++)
                {
                    if (NormalFoodAge[i] > 100)
                    {
                        DeleteElement(50, NormalFoodAge, i);
                    }
                }
                player->Energy = player->FullEnergy;
            }
            else
            {
                for (int i = 0; i < 50; i++)
                {
                    if (NormalFoodAge[i] <= 100)
                    {
                        DeleteElement(50, NormalFoodAge, i);
                        break;
                    }
                }
                player->HP += 5;
                player->foodinventory[NORMALFOOD]--;
                player->Energy = player->FullEnergy;
            }
            break;
        case '2':
            if (player->foodinventory[FINEFOOD] == 0)
                break;
            player->HP += 10;
            player->foodinventory[FINEFOOD]--;
            strength = 2;
            DeleteElement(50, FineFoodAge, 0);
            player->Energy = player->FullEnergy;
            break;
        case '3':
            if (player->foodinventory[MAGICFOOD] == 0)
                break;
            player->HP += 10;
            player->foodinventory[MAGICFOOD]--;
            speed = 2;
            DeleteElement(50, MagicFoodAge, 0);
            player->Energy = player->FullEnergy;
            break;
        case 'q':
            state = 0;
            break;
        default:
            break;
        }
        if (player->HP > player->FullHP)
        {
            player->HP = player->FullHP;
        }
    }
    return;
}

void LoadSpellInventory(Player *player)
{
    clear();
    int b, state = 1;
    while (state)
    {
        mvprintw(ScreenHeight / 2 - 2, ScreenWidth / 2 - 6, "1- Health spell: %d", ((player->spellinventory[0])));
        mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 6, "2- Speed spell: %d", player->spellinventory[1]);
        mvprintw(ScreenHeight / 2 + 2, ScreenWidth / 2 - 6, "3- Damage spell: %d", player->spellinventory[2]);
        b = getch();
        switch (b)
        {
        case '1':
            if (player->spellinventory[0] == 0)
                break;
            else
                player->HP = player->FullHP;
            player->spellinventory[0]--;
            break;
        case '2':
            if (player->spellinventory[1] == 0)
                break;
            player->spellinventory[1]--;
            speed = 2;
            break;
        case '3':
            if (player->spellinventory[2] == 0)
                break;
            player->spellinventory[2]--;
            strength = 2;
            break;
        case 'q':
            state = 0;
            break;
        default:
            break;
        }
    }
    return;
}

void LoadWeaponInventory(Player *player)
{
    clear();
    int b, state = 1;
    while (state)
    {
        mvprintw(ScreenHeight / 2 - 4, ScreenWidth / 2 - 20, "1- \u2692: Mace:             -        5");
        mvprintw(ScreenHeight / 2 - 2, ScreenWidth / 2 - 20, "2- d: Dagger:           5        12");
        mvprintw(ScreenHeight / 2, ScreenWidth / 2 - 20, "3- m: Magic Wand:       10       15");
        mvprintw(ScreenHeight / 2 + 2, ScreenWidth / 2 - 20, "4- \u27B3: Normal Arrow:     5        5");
        mvprintw(ScreenHeight / 2 + 4, ScreenWidth / 2 - 20, "5- \u2694: Sword:            -        10");
        mvprintw(ScreenHeight / 2 - 6, ScreenWidth / 2 + 2, "Range    Damage   Count");
        mvprintw(1, 2, "press q to quit");
        for (int i = 0; i < 4; i++)
        {
            mvprintw(ScreenHeight / 2 - 4 + (i * 2), ScreenWidth / 2 + 22, "%d", player->weapninventory[i]);
            if (i == player->wielded)
            {
                mvprintw(ScreenHeight / 2 - 4 + (i * 2), ScreenWidth / 2 - 23, "\u2705");
            }
            else
            {
                mvprintw(ScreenHeight / 2 - 4 + (i * 2), ScreenWidth / 2 - 23, " ");
            }
        }
        refresh();
        b = getch();
        if (player->wielded == HAND)
        {
            switch (b)
            {
            case '1':
                player->wielded = MACE;
                mvprintw(2, 2, "Default weapon changed to Mace.               ");

                break;
            case '2':
                if (player->weapninventory[DAGGER])
                {
                    player->wielded = DAGGER;
                    mvprintw(2, 2, "Default weapon changed to Dagger.                ");
                }
                else
                {
                    mvprintw(2, 2, "You don't have this weapon!                    ");
                }
                break;
            case '3':
                if (player->weapninventory[MAGICWAND])
                {
                    player->wielded = MAGICWAND;
                    mvprintw(2, 2, "Default weapon changed to Magicwand.            ");
                }
                else
                {
                    mvprintw(2, 2, "You don't have this weapon!               ");
                }
                break;
            case '4':
                if (player->weapninventory[ARROW])
                {
                    player->wielded = ARROW;
                    mvprintw(2, 2, "Default weapon changed to ARROW.              ");
                }
                else
                {
                    mvprintw(2, 2, "You don't have this weapon!              ");
                }
                break;
            case '5':
                if (player->weapninventory[SWORD])
                {
                    player->wielded = SWORD;
                    mvprintw(2, 2, "Default weapon changed to Sword.           ");
                }
                else
                {
                    mvprintw(2, 2, "You don't have this weapon!               ");
                }
                break;
            case 'q':
                state = 0;
                break;
            default:
                mvprintw(2, 2, "Invalid input!                             ");
                break;
            }
        }
        else
        {
            switch (b)
            {
            case '1' ... '4':
                mvprintw(2, 2, "You\'re wielding a weapon already!         ");
                break;
            case 'w':
                player->wielded = HAND;
                mvprintw(2, 2, "Weapon has been placed in your inventory");
                break;
            case 'q':
                state = 0;
                for (int i = 0; i < 50; i++)
                    mvprintw(2, i, " ");
                mvprintw(1, 2, "                ");
                break;
            default:
                mvprintw(2, 2, "Invalid input!                             ");
                break;
            }
        }
        refresh();
    }
}

void CheckDoor(Room **rooms, char **map, int RoomCount, int y, int x)
{
    for (int i = 0; i < RoomCount; i++)
    {
        if (((x == rooms[i]->x1 || x == rooms[i]->x2) && (y > rooms[i]->y1 && y < rooms[i]->y2)) || ((y == rooms[i]->y1 || y == rooms[i]->y2) && (x > rooms[i]->x1 && x < rooms[i]->x2)))
        {
            rooms[i]->show = 1;
        }
    }
}

int CheckCo(Player *player, char **map, int y, int x)
{
    int gold; // gold worth
    int chance;
    switch (map[y][x])
    {
    case '.':
    case '+':
    case '#':
    case '>':
    case '<':
        for (int i = 0; i < 50; i++)
            mvprintw(1, i + 1, " ");
        return 1;
        break;
    case '*':
        attron(COLOR_PAIR(1));
        mvprintw(1, 1, "GOLD!");
        attroff(COLOR_PAIR(1));
        for (int i = 6; i < 50; i++)
            mvprintw(1, i, " ");
        if (rand() % 5 == 0)
            gold = 7 + (rand() % 4);
        else if (rand() % 5 == 1)
            gold = 1 + rand() % 3;
        else
            gold = 4 + (rand() % 4);
        player->Gold += gold * Level;
        player->score += 2 * gold * Level;
        return 1;
        break;
    case '3':
        attron(COLOR_PAIR(4));
        mvprintw(1, 1, "Diamond!");
        attroff(COLOR_PAIR(4));
        for (int i = 9; i < 50; i++)
            mvprintw(1, i, " ");
        gold; // gold worth
        if (rand() % 5 == 0)
            gold = 7 + (rand() % 4);
        else if (rand() % 5 == 1)
            gold = 1 + rand() % 3;
        else
            gold = 4 + (rand() % 4);
        player->Gold += gold * Level * 5;
        player->score += 2 * gold * Level * 5;
        return 1;
        break;
    case FOOD_SYMBOL: // add food to inventory
        attron(COLOR_PAIR(5));
        mvprintw(1, 1, "FOOD!");
        attroff(COLOR_PAIR(5));
        for (int i = 6; i < 50; i++)
            mvprintw(1, i, " ");
        chance = rand() % 4;
        if (chance % 4 == 0)
        {
            player->foodinventory[MAGICFOOD]++;
        }
        else if (chance % 4 == 1)
        {
            player->foodinventory[FINEFOOD]++;
        }
        else
        {
            player->foodinventory[NORMALFOOD]++;
        }
        return 1;
    case SPELL_SYMBOL: // add spells to inventory
        attron(COLOR_PAIR(5));
        mvprintw(1, 1, "SPELL!");
        attroff(COLOR_PAIR(5));
        for (int i = 7; i < 50; i++)
            mvprintw(1, i, " ");
        chance = rand() % 3;
        if (chance % 3 == 0)
        {
            player->spellinventory[0]++;
        }
        else if (chance % 3 == 1)
        {
            player->spellinventory[1]++;
        }
        else
        {
            player->spellinventory[2]++;
        }
        return 1;
    case MACE:
        player->weapninventory[MACE] = 1;
        return 1;
        break;
    case SWORD:
        player->weapninventory[SWORD] = 1;
        return 1;
        break;
    case DAGGER:
        player->weapninventory[DAGGER] += 10;
        return 1;
        break;
    case ARROW:
        player->weapninventory[ARROW] += 20;
        return 1;
        break;
    case MAGICWAND:
        player->weapninventory[MAGICWAND] += 8;
        return 1;
        break;
    case '|':
    case '-':
    case ' ':
    case 'O':
        mvprintw(1, 1, "you hit an obstacle dummy; can't move there!");
        return 0;
        break;
    case TREASURE:
        player->treasure = 1;
        player->Gold += 1000;
        player->score += 2000;
        return 1;
    default:
        return 0;
        break;
    }
    return 0;
}

void MapInit(char ***map, Room **rooms, int RoomCount, Room *StairRoom)
{
    int chance = Level * Scarcity;
    static int Repeat = 0;
    if (StairRoom != NULL)
    {
        Repeat = 0;
    }
    if (Repeat)
    {
        for (int i = 0; i < RoomCount; i++)
        {
            free(rooms[i]);
            rooms[i] = NULL;
        }
    }
    Repeat++;
    for (int i = 0; i < ScreenHeight; i++)
    {
        for (int j = 0; j < ScreenWidth; j++)
        {
            (*map)[i][j] = ' ';
        }
    }

    for (int i = 0; i < RoomCount; i++)
    {
        if (StairRoom && (i == 0))
        {
            rooms[i] = StairRoom;
            DrawRoom(*map, rooms[i]);
            continue;
        }
        rooms[i] = MakeRoom(*map);
    }

    for (int i = 0; i < RoomCount - 1; i++)
    {
        ConnectRooms(map, rooms, rooms[i], rooms[i + 1], RoomCount, StairRoom);
    }
    Repeat = 0;
    for (int i = 0; i < RoomCount - 1; i++)
    {
        int y1 = rooms[i]->y1;
        int x1 = rooms[i]->x1;
        int y2 = rooms[i]->y2;
        int x2 = rooms[i]->x2;
        for (int j = x1; j < x2; j++)
        {
            if ((*map)[y1][j] == '+' && (*map)[y1 - 1][j] != '#')
            {
                (*map)[y1][j] = '-';
            }
            if ((*map)[y2][j] == '+' && (*map)[y2 + 1][j] != '#')
            {
                (*map)[y2][j] = '-';
            }
        }
        for (int j = y1; j < y2; j++)
        {
            if (x1 - 1 > LeftLimit)
            {
                if ((*map)[j][x1] == '+' && (*map)[j][x1 - 1] != '#')
                {
                    (*map)[j][x1] = '|';
                }
            }
            if (x1 + 1 < RightLimit)
            {
                if ((*map)[j][x2] == '+' && (*map)[j][x2 + 1] != '#')
                {
                    (*map)[j][x2] = '|';
                }
            }
        }
        (*map)[y1][x1] = '-';
        (*map)[y1][x2] = '-';
        (*map)[y2][x1] = '-';
        (*map)[y2][x2] = '-';
        if (rooms[i]->type == 2)
        {
            (*map)[(rooms[i]->y1 + rooms[i]->y2) / 2][(rooms[i]->x1 + rooms[i]->x2) / 2] = TREASURE;
        }
    }
    for (int i = 2; i < ScreenHeight; i++)
    {
        for (int j = 0; j < ScreenWidth; j++)
        {

            // gold drop chance.

            if ((*map)[i][j] == '.' && !(rand() % (Difficulty * GOLD_DROP / chance * Difficulty)))
            {
                (*map)[i][j] = '*';
            }

            if ((*map)[i][j] == '.' && !(rand() % (GOLD_DROP / chance * 10 * Difficulty)))
            {
                (*map)[i][j] = '3';
            }

            if ((*map)[i][j] == '.' && !(rand() % (FOOD_DROP / chance) * 3 * Difficulty))
            {
                (*map)[i][j] = FOOD_SYMBOL;
            }

            // Window
            if ((*map)[i][j] == '.' && !(rand() % 40))
            {
                int door = 0;
                for (int x = -1; x <= 1; x++)
                {
                    if ((*map)[i + x][j] == '+' || (*map)[i][j] == '=')
                    {
                        door = 1;
                    }
                    if ((*map)[i][j + x] == '+' || (*map)[i][j] == '=')
                    {
                        door = 1;
                    }
                }
                if (door == 0)
                {
                    (*map)[i][j] = 'O';
                }
                door = 0;
            }
            if (((*map)[i][j] == '|' || ((*map)[i][j] == '-')) && (!(rand() % 30)))
            {
                (*map)[i][j] = '=';
            }

            // spawn weapons
            if ((*map)[i][j] == '.' && !(rand() % ((WEAPON_DROP) / chance * Difficulty)))
            {
                (*map)[i][j] = DAGGER;
            }
            else if ((*map)[i][j] == '.' && !(rand() % ((WEAPON_DROP * 2) / chance * Difficulty)))
            {
                (*map)[i][j] = MAGICWAND;
            }
            else if ((*map)[i][j] == '.' && !(rand() % ((WEAPON_DROP) / chance * Difficulty)))
            {
                (*map)[i][j] = ARROW;
            }
            else if ((*map)[i][j] == '.' && !(rand() % ((WEAPON_DROP * 2) / chance * Difficulty)))
            {
                (*map)[i][j] = SWORD;
            }

            // Spells
            if ((*map)[i][j] == '.' && !(rand() % ((SpellDrop) / chance * Difficulty)))
            {
                (*map)[i][j] = SPELL_SYMBOL;
            }
            // monsters
            if ((*map)[i][j] == '.' && !(rand() % (MONSTER_SPAWN_RATE / chance / Difficulty)))
            {
                (*map)[i][j] = DEAMON;
            }
            else if ((*map)[i][j] == '.' && !(rand() % (MONSTER_SPAWN_RATE / chance / Difficulty)))
            {
                (*map)[i][j] = FIRE_BREATHING_MONSTER;
            }
            else if ((*map)[i][j] == '.' && !(rand() % (MONSTER_SPAWN_RATE / chance / Difficulty)))
            {
                (*map)[i][j] = GIANT;
            }
            else if ((*map)[i][j] == '.' && !(rand() % (MONSTER_SPAWN_RATE / chance / Difficulty)))
            {
                (*map)[i][j] = SNAKE;
            }
            else if ((*map)[i][j] == '.' && !(rand() % (MONSTER_SPAWN_RATE / chance / Difficulty)))
            {
                (*map)[i][j] = UNDEAD;
            }
        }
    }
    for (int i = 0; i < RoomCount; i++)
    {
        if (rooms[i]->type == 1)
        {
            for (int y = rooms[i]->y1; y < rooms[i]->y2; y++)
            {
                for (int x = rooms[i]->x1; x < rooms[i]->x2; x++)
                {
                    if (!(rand() % (SpellDrop / 50)) && (*map)[y][x] == '.')
                    {
                        (*map)[y][x] = SPELL_SYMBOL;
                    }
                }
            }
        }
    }
    if (StairRoom != NULL)
    {
        int c = 0;
        for (int y = StairRoom->y1; y <= StairRoom->y2; y++)
        {
            int a = (*map)[y][StairRoom->x1];
            int b = (*map)[y][StairRoom->x2];
            if (a != '+' && a != '=' && a != '|' && a != '-' && b != '+' && b != '=' && b != '|' && b != '-')
            {
                c++;
            }
        }
        for (int x = StairRoom->x1; x <= StairRoom->x2; x++)
        {
            int a = (*map)[StairRoom->y1][x];
            int b = (*map)[StairRoom->y2][x];
            if (a != '+' && a != '=' && a != '|' && a != '-' && b != '+' && b != '=' && b != '|' && b != '-')
            {
                c++;
            }
        }
        if (c)
        {
            mvprintw(1, 1, "backayaro it ran press a button");
            getch();
            MapInit(map, rooms, RoomCount, StairRoom);
        }
        refresh();
    }
    return;
}

Room *MakeRoom(char **map)
{
    int x1, y1, x2, y2, width, length, distance = 3;
    static int TreasureRoom = 0;
    x1 = LeftLimit + (rand() % (RightLimit - LeftLimit - MaximumRoomW - distance));
    y1 = UpLimit + (rand() % (DownLimit - UpLimit - MaximumRoomL - distance));
    width = MinimumRoomW + (rand() % (MaximumRoomW - MinimumRoomW + 1));
    x2 = x1 + width;
    length = MinimumRoomL + (rand() % (MaximumRoomL - MinimumRoomL + 1));
    y2 = y1 + length;
    int Roomablity = 1;
    for (int i = y1 - distance; i <= y2 + distance; i++)
    {
        for (int j = x1 - distance; j <= x2 + distance; j++)
        {
            if (map[i][j] != ' ')
            {
                Roomablity = 0;
            }
        }
    }
    if (Roomablity)
    {
        Room *room = (Room *)malloc(sizeof(Room));
        room->x1 = x1;
        room->x2 = x2;
        room->y1 = y1;
        room->y2 = y2;
        room->show = 0;
        room->type = 0;
        // Treasure room
        if (Level == 4 && TreasureRoom == 0 && !(rand() % 2))
        {
            room->type = 2;
            TreasureRoom++;
        }
        else if (!(rand() % 15))
        {
            room->type = 1;
        }
        DrawRoom(map, room);
        return room;
    }
    else
    {
        return MakeRoom(map);
    }
}

void DrawRoom(char **map, Room *room)
{
    for (int i = room->y1; i <= room->y2; i++)
    {
        for (int j = room->x1; j <= room->x2; j++)
        {
            if (i == room->y1 || i == room->y2)
            {
                map[i][j] = '-';
            }
            else if (j == room->x1 || j == room->x2)
            {
                map[i][j] = '|';
            }
            else
            {
                map[i][j] = '.';
            }
        }
    }
    return;
}

void ConnectRooms(char ***map, Room **rooms, Room *room1, Room *room2, int RoomCount, Room *StairRoom)
{
    srand(time(NULL));
    clock_t StartTime, CurrentTime;
    double ElapsedTime;
    int x1, y1, x2, y2;
    x1 = (room1->x1 + room1->x2) / 2;
    y1 = (room1->y1 + room1->y2) / 2;
    x2 = (room2->x1 + room2->x2) / 2;
    y2 = (room2->y1 + room2->y2) / 2;
    StartTime = clock();
    while (y1 != y2)
    {
        CurrentTime = clock();
        ElapsedTime = (((double)CurrentTime - StartTime) / CLOCKS_PER_SEC);
        if (ElapsedTime > TIMELIMIT)
        {
            clear();
            MapInit(map, rooms, RoomCount, NULL);
            return;
        }
        DrawCorridor((*map), x1, y1);
        if (y1 > y2)
        {
            if ((*map)[y1 - 1][x1] == '-' && (*map)[y1 - 2][x1] == '|')
            {
                if ((*map)[y1 - 1][x1 + 1] == ' ')
                {
                    x1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 - 1][x1 - 1] == ' ')
                {
                    x1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                y1--;
                DrawCorridor((*map), x1, y1);
            }
        }
        else if (y1 < y2)
        {
            if ((*map)[y1 + 1][x1] == '-' && (*map)[y1 + 2][x1] == '|')
            {
                if ((*map)[y1 + 1][x1 + 1] == ' ')
                {
                    x1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 + 1][x1 - 1] == ' ')
                {
                    x1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                y1++;
                DrawCorridor((*map), x1, y1);
            }
        }
        DrawCorridor((*map), x1, y1);
        while (!(rand() % 2))
        {
            CurrentTime = clock();
            ElapsedTime = (((double)CurrentTime - StartTime) / CLOCKS_PER_SEC);
            if (ElapsedTime > TIMELIMIT)
            {
                clear();
                MapInit(map, rooms, RoomCount, StairRoom);
                return;
            }
            if ((x1 > x2) && (*map)[y1][x1 - 1] != '|' && (*map)[y1][x1 - 1] != '-')
            {
                x1--;
                DrawCorridor((*map), x1, y1);
            }
            else if ((x1 < x2) && (*map)[y1][x1 + 1] != '|' && (*map)[y1][x1 + 1] != '-')
            {
                x1++;
                DrawCorridor((*map), x1, y1);
            }
        }
    }
    while (x1 != x2)
    {
        CurrentTime = clock();
        ElapsedTime = (((double)CurrentTime - StartTime) / CLOCKS_PER_SEC);
        if (ElapsedTime > TIMELIMIT)
        {
            clear();
            MapInit(map, rooms, RoomCount, StairRoom);
            return;
        }
        DrawCorridor((*map), x1, y1);
        if (x1 > x2)
        {
            if (((*map)[y1][x1 - 1] == '-' && (*map)[y1 - 1][x1 - 1] == '|') || ((*map)[y1][x1 - 1] == '-' && (*map)[y1 + 1][x1 - 1] == '|'))
            {
                if ((*map)[y1 + 1][x1 - 1] == ' ')
                {
                    y1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 - 1][x1 - 1] == ' ')
                {
                    y1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                x1--;
                DrawCorridor((*map), x1, y1);
            }
        }
        else if (x1 < x2)
        {
            if (((*map)[y1][x1 + 1] == '-' && (*map)[y1 - 1][x1 + 1] == '|') || ((*map)[y1][x1 + 1] == '-' && (*map)[y1 + 1][x1 + 1] == '|'))
            {
                if ((*map)[y1 + 1][x1 + 1] == ' ')
                {
                    y1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 - 1][x1 + 1] == ' ')
                {
                    y1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                x1++;
                DrawCorridor((*map), x1, y1);
            }
        }
    }
    while (y1 != y2)
    {
        CurrentTime = clock();
        ElapsedTime = (((double)CurrentTime - StartTime) / CLOCKS_PER_SEC);
        if (ElapsedTime > TIMELIMIT)
        {
            clear();
            MapInit(map, rooms, RoomCount, StairRoom);
            return;
        }
        DrawCorridor((*map), x1, y1);
        if (y1 > y2)
        {
            if ((*map)[y1 - 1][x1] == '-' && (*map)[y1 - 2][x1] == '|')
            {
                if ((*map)[y1 - 1][x1 + 1] == ' ')
                {
                    x1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 - 1][x1 - 1] == ' ')
                {
                    x1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                y1--;
                DrawCorridor((*map), x1, y1);
            }
        }
        else if (y1 < y2)
        {
            if ((*map)[y1 + 1][x1] == '-' && (*map)[y1 + 2][x1] == '|')
            {
                if ((*map)[y1 + 1][x1 + 1] == ' ')
                {
                    x1++;
                    DrawCorridor((*map), x1, y1);
                }
                else if ((*map)[y1 + 1][x1 - 1] == ' ')
                {
                    x1--;
                    DrawCorridor((*map), x1, y1);
                }
            }
            else
            {
                y1++;
                DrawCorridor((*map), x1, y1);
            }
        }
        DrawCorridor((*map), x1, y1);
        while (!(rand() % 2))
        {
            CurrentTime = clock();
            ElapsedTime = (((double)CurrentTime - StartTime) / CLOCKS_PER_SEC);
            if (ElapsedTime > TIMELIMIT)
            {
                clear();
                MapInit(map, rooms, RoomCount, StairRoom);
                return;
            }

            if ((x1 > x2) && (*map)[y1][x1 - 1] != '|' && (*map)[y1][x1 - 1] != '-')
            {
                x1--;
                DrawCorridor((*map), x1, y1);
            }
            else if ((x1 < x2) && (*map)[y1][x1 + 1] != '|' && (*map)[y1][x1 + 1] != '-')
            {
                x1++;
                DrawCorridor((*map), x1, y1);
            }
        }
    }
    return;
}

void DrawCorridor(char **map, int x, int y)
{
    if (map[y][x] == ' ')
    {
        map[y][x] = '#';
    }
    else if (map[y][x] == '|' || map[y][x] == '-')
    {
        map[y][x] = '+';
    }
    return;
}

int SpawnPlayer(Room **rooms, int RoomCount, Player *player, char **map)
{
    int RoomIndex = rand() % RoomCount, x, y;
    Room *SpawnRoom = rooms[RoomIndex];
    SpawnRoom->show = 1;
    do
    {
        x = SpawnRoom->x1 + 1 + (rand() % (SpawnRoom->x1 + 1 - SpawnRoom->x2 - 1 + 1));
        y = SpawnRoom->y1 + 1 + (rand() % (SpawnRoom->y1 + 1 - SpawnRoom->y2 - 1 + 1));
    } while (map[y][x] != '.');
    player->x = x;
    player->y = y;
    map[y][x] = PlayerSymbol;
    return RoomIndex;
}

int SpawnStairs(char **map, Room **rooms, int RoomCount, int SpawnRoom)
{
    int StairIndex, x, y;
    do
    {
        StairIndex = rand() % RoomCount;
    } while (StairIndex == SpawnRoom);
    int x1 = rooms[StairIndex]->x1, x2 = rooms[StairIndex]->x2, y1 = rooms[StairIndex]->y1, y2 = rooms[StairIndex]->y2;
    do
    {
        x = x1 + 1 + (rand() % (x2 - x1));
        y = y1 + 1 + (rand() & (y2 - y1));
    } while (map[y][x] != '.');

    map[y][x] = '>';

    return StairIndex;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int compareParticipants(const void *a, const void *b)
{
    Participant *participantA = (Participant *)a;
    Participant *participantB = (Participant *)b;
    return participantB->gold - participantA->gold;
}

void sortParticipantsByGold(Participant *participants, int count)
{
    qsort(participants, count, sizeof(Participant), compareParticipants);
}

void ScoreBoard()
{
    clear();
    FILE *UsernameFile = fopen("user data/usernames.txt", "r");
    if (UsernameFile == NULL)
    {
        perror("Error opening usernames file");
        return;
    }

    int LineCount = 0;
    char ch;
    while ((ch = fgetc(UsernameFile)) != EOF)
    {
        if (ch == '\n')
        {
            LineCount++;
        }
    }
    fseek(UsernameFile, 0, SEEK_SET);

    if (LineCount == 0)
    {
        printf("No usernames found.\n");
        fclose(UsernameFile);
        return;
    }

    Participant *participants = (Participant *)malloc(LineCount * sizeof(Participant));
    if (participants == NULL)
    {
        perror("Memory allocation failed");
        fclose(UsernameFile);
        return;
    }

    char Username[LineCount][50];

    for (int i = 0; i < LineCount; i++)
    {
        if (fgets(Username[i], sizeof(Username[i]), UsernameFile))
        {
            Username[i][strcspn(Username[i], "\n")] = '\0';
        }
    }

    fclose(UsernameFile);

    for (int i = 0; i < LineCount; i++)
    {
        strcpy(participants[i].name, Username[i]);
    }

    char filename[80];
    for (int i = 0; i < LineCount; i++)
    {

        sprintf(filename, "user data/%s/score.txt", Username[i]);
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            perror("Error opening score file");
            continue;
        }
        fscanf(file, "%d", &participants[i].gold);
        fscanf(file, "%d", &participants[i].score);
        fscanf(file, "%d", &participants[i].gamesplayed);
        fclose(file);
    }
    init_color(9, 1000, 800, 0);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, 9, COLOR_BLACK);
    init_pair(6, 10, COLOR_BLACK);
    init_pair(7, 11, COLOR_BLACK);

    sortParticipantsByGold(participants, LineCount);
    mvprintw(7, 50, "Rank     Player");
    mvprintw(7, 102, "Gold              Score          Experience");
    int x = 0;
    for (int i = 0; i < LineCount; i++)
    {
        if (logged && strcmp(User, participants[i].name) == 0)
        {
            attron(A_BOLD);
            attron(A_ITALIC);
            attron(COLOR_PAIR(5));
        }
        mvprintw(i + 10, 50, " %2d      %-50s", i + 1, participants[i].name);
        if (i == 0)
        {
            attron(COLOR_PAIR(YELLOW));
            mvprintw(i + 10, 50, " %2d      \U0001F947%-50s", i + 1, participants[i].name);
            mvprintw(i + 10, 25, "Super Duper Epic Gamer");
        }
        else if (i == 1)
        {
            attron(COLOR_PAIR(BLUE));
            mvprintw(i + 10, 50, " %2d      \U0001F948%-50s", i + 1, participants[i].name);
            mvprintw(i + 10, 25, "Super Epic Gamer");
        }
        else if (i == 2)
        {
            attron(COLOR_PAIR(RED));
            mvprintw(i + 10, 50, " %2d      \U0001F949%-50s", i + 1, participants[i].name);
            mvprintw(i + 10, 25, "Epic Gamer");
        }

        for (int j = 61 + strlen(participants[i].name); j < 100; j++)
            mvprintw(i + 10, j, ".");

        mvprintw(i + 10, 100, " %5d RC .......... %5d ..........%3d", participants[i].gold, participants[i].score, participants[i].gamesplayed);
        if (logged && strcmp(User, participants[i].name) == 0)
        {
            attroff(A_BOLD);
            attroff(COLOR_PAIR(1));
            attroff(A_ITALIC);
        }
        if (i == 0)
        {
            mvprintw(i + 10, 25, "(Super Duper Epic Gamer)");
            attroff(COLOR_PAIR(YELLOW));
        }
        else if (i == 1)
        {
            mvprintw(i + 10, 25, "(Super Epic Gamer)");
            attroff(COLOR_PAIR(BLUE));
        }
        else if (i == 2)
        {
            mvprintw(i + 10, 25, "(Epic Gamer)");
            attroff(COLOR_PAIR(RED));
        }
        x++;
    }
    for (int i = x; i < 30; i++)
    {
        for (int j = 59; j < 137; j++)
        {
            mvprintw(i + 10, j  , ".");
        }
    }
    refresh();
    getch();

    free(participants);
    return;
}
