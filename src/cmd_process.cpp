#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>
#include <fcntl.h>
#include <ncurses.h>

#include "cmd_process.h"
#include "action.h"

#include <nlohmann/json.hpp>

int Col = STP7_COL;
int Row = ID_1_ROW;
int Old_Col;
int Old_Row;
bool bBeginCommandMode = false;
bool bEdited = false;
int indexPage = 1;
Action::PAGE Page;
Action::STEP Step;


int _getch()
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

struct termios oldterm, new_term;
void set_stdin(void)
{
	tcgetattr(0,&oldterm);
	new_term = oldterm;
	new_term.c_lflag &= ~(ICANON | ECHO | ISIG); // �ǹ̴� struct termios�� ã���� ��.
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_term);
}

void reset_stdin(void)
{
	tcsetattr(0, TCSANOW, &oldterm);
}

bool AskSave()
{
	if(bEdited == true)
	{
		PrintCmd("Are you sure? (y/n)");
		if(_getch() != 'y')
		{
			ClearCmd();
			return true;
		}
	}

	return false;
}


void GoToCursor(int col, int row)
{
	char *cursor;
	char *esc_sequence;
	cursor = tigetstr("cup");
	esc_sequence = tparm(cursor, row, col);
	putp(esc_sequence);

	Col = col;
	Row = row;
}

void MoveUpCursor()
{
	if(Col >= STP7_COL && Col <= CCWSLOPE_COL)
	{
		if( Row > ID_1_ROW )
			GoToCursor(Col, Row-1);
	}
	else
	{
		if( Row > PLAYCOUNT_ROW )
			GoToCursor(Col, Row-1);
	}
}

void MoveDownCursor()
{
	if(Col >= STP7_COL && Col <= STP6_COL)
	{
		if( Row < SPEED_ROW )
			GoToCursor(Col, Row+1);
	}
	else if(Col <= CCWSLOPE_COL)
	{
		if( Row < ID_22_ROW )
			GoToCursor(Col, Row+1);
	}
	else
	{
		if( Row < EXIT_ROW )
			GoToCursor(Col, Row+1);
	}
}

void MoveLeftCursor()
{
	switch(Col)
	{
	case STP0_COL:
		GoToCursor(STP7_COL, Row);
		break;

	case STP1_COL:
		GoToCursor(STP0_COL, Row);
		break;

	case STP2_COL:
		GoToCursor(STP1_COL, Row);
		break;

	case STP3_COL:
		GoToCursor(STP2_COL, Row);
		break;

	case STP4_COL:
		GoToCursor(STP3_COL, Row);
		break;

	case STP5_COL:
		GoToCursor(STP4_COL, Row);
		break;

	case STP6_COL:
		GoToCursor(STP5_COL, Row);
		break;

	case CWSLOPE_COL:
		GoToCursor(STP6_COL, Row);
		break;

	case CCWSLOPE_COL:
		GoToCursor(CWSLOPE_COL, Row);
		break;

	case PAGEPARAM_COL:
		GoToCursor(CCWSLOPE_COL, Row);
		break;
	}
}

void MoveRightCursor()
{
	switch(Col)
	{
	case STP7_COL:
		GoToCursor(STP0_COL, Row);
		break;

	case STP0_COL:
		GoToCursor(STP1_COL, Row);
		break;

	case STP1_COL:
		GoToCursor(STP2_COL, Row);
		break;

	case STP2_COL:
		GoToCursor(STP3_COL, Row);
		break;

	case STP3_COL:
		GoToCursor(STP4_COL, Row);
		break;

	case STP4_COL:
		GoToCursor(STP5_COL, Row);
		break;

	case STP5_COL:
		GoToCursor(STP6_COL, Row);
		break;

	case STP6_COL:
		GoToCursor(CWSLOPE_COL, Row);
		break;

	case CWSLOPE_COL:
		GoToCursor(CCWSLOPE_COL, Row);
		break;

	case CCWSLOPE_COL:
		if(Row >= PLAYCOUNT_ROW && Row <= EXIT_ROW)
			GoToCursor(PAGEPARAM_COL, Row);
		break;
	}
}

void DrawIntro()
{
	int nrows, ncolumns;
    setupterm(NULL, fileno(stdout), (int *)0);
    nrows = tigetnum("lines");
    ncolumns = tigetnum("cols");

	system("clear");
	printf("\n");
	printf("[Action Editor for DARwIn %s]\n", PROGRAM_VERSION);
	printf("\n");
	printf(" *Terminal screen size must be %d(col)x%d(row).\n", SCREEN_COL, SCREEN_ROW);
    printf(" *Current terminal has %d columns and %d rows.\n", ncolumns, nrows);
	printf("\n");
	printf("\n");
	printf("Press any key to start program...\n");
	_getch();

	Action::GetInstance()->LoadPage(indexPage, &Page);

	DrawPage();
}

void DrawEnding()
{
	system("clear");
	printf("\n");
	printf("Terminate Action Editor");
	printf("\n");
}

void DrawPage()
{
	int old_col = Col;
	int old_row = Row;

	system("clear");
	// 80    01234567890123456789012345678901234567890123456789012345678901234567890123456789     //24
	printf( "ID: 1(R_SHO_PITCH)[    ]                                                       \n" );//0
	printf( "ID: 2(L_SHO_PITCH)[    ]                                       Page Number:    \n" );//1
	printf( "ID: 3(R_SHO_ROLL) [    ]                                        Address:       \n" );//2
	printf( "ID: 4(L_SHO_ROLL) [    ]                                         Play Count:   \n" );//3
	printf( "ID: 5(R_ELBOW)    [    ]                                          Page Step:   \n" );//4
	printf( "ID: 6(L_ELBOW)    [    ]                                         Page Speed:   \n" );//5
	printf( "ID: 7(R_HIP_YAW)  [    ]                                         Accel Time:   \n" );//6
	printf( "ID: 8(L_HIP_YAW)  [    ]                                       Link to Next:   \n" );//7
	printf( "ID: 9(R_HIP_ROLL) [    ]                                       Link to Exit:   \n" );//8
	printf( "ID:10(L_HIP_ROLL) [    ]                                                       \n" );//9
	printf( "ID:11(R_HIP_PITCH)[    ]                                                       \n" );//0
	printf( "ID:12(L_HIP_PITCH)[    ]                                                       \n" );//1
	printf( "ID:13(R_KNEE)     [    ]                                                       \n" );//2
	printf( "ID:14(L_KNEE)     [    ]                                                       \n" );//3
	printf( "ID:15(R_ANK_PITCH)[    ]                                                       \n" );//4
	printf( "ID:16(L_ANK_PITCH)[    ]                                                       \n" );//5
	printf( "ID:17(R_ANK_ROLL) [    ]                                                       \n" );//6
	printf( "ID:18(L_ANK_ROLL) [    ]                                                       \n" );//7
	printf( "ID:19(HEAD_PAN)   [    ]                                                       \n" );//8
	printf( "ID:20(HEAD_TILT)  [    ]                                                       \n" );//9
	printf( "ID:21(R_GRIPPER)  [    ]                                                       \n" );//10
	printf( "ID:22(L_GRIPPER)  [    ]                                                       \n" );//11
	printf( "   PauseTime      [    ]                                                       \n" );//0

	if( Page.header.schedule == Action::SPEED_BASE_SCHEDULE )
		printf( "   Speed          [    ]                                                       \n" );//1
	else if( Page.header.schedule == Action::TIME_BASE_SCHEDULE )
		printf( "   Time(x 8msec)  [    ]                                                       \n" );//1
	
	printf( "                   STP7  STP0 STP1 STP2 STP3 STP4 STP5 STP6                    \n" );//2
	printf( "]                                                                              " );  // 3

	for(int i=0; i<=Action::MAXNUM_STEP; i++ )
		DrawStep(i);

	// Draw Compliance slope
	for( int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++ )
	{
		GoToCursor(CWSLOPE_COL, id -1);
		printf( "%.1d%.1d", Page.header.slope[id]>>4, Page.header.slope[id]&0x0f );
	}

	// Draw Page parameter
	GoToCursor( PAGEPARAM_COL, PLAYCOUNT_ROW );
	printf( "%.3d", Page.header.repeat );

	GoToCursor( PAGEPARAM_COL, STEPNUM_ROW );
	printf( "%.3d", Page.header.stepnum );
		
	GoToCursor( PAGEPARAM_COL, PLAYSPEED_ROW );
	printf( "%.3d", Page.header.speed );

	GoToCursor( PAGEPARAM_COL, ACCEL_ROW );
	printf( "%.3d", Page.header.accel );

	GoToCursor( PAGEPARAM_COL, NEXT_ROW );
	printf( "%.3d", Page.header.next );

	GoToCursor( PAGEPARAM_COL, EXIT_ROW );
	printf( "%.3d", Page.header.exit );

	// Draw Page information
	DrawName();

	GoToCursor(PAGENUM_COL, PAGENUM_ROW);
	printf( "%.4d", indexPage );

	GoToCursor(ADDR_COL, ADDR_ROW);
	printf( "0x%.5X", (int)(indexPage*sizeof(Action::PAGE)) );

	DrawStepLine(false);

	GoToCursor(old_col, old_row);
}

void DrawStep(int index)
{
	int old_col = Col;
	int old_row = Row;
	Action::STEP *step;
	int col;

	switch(index)
	{
	case 0:
		col = STP0_COL;
		step = &Page.step[0];
		break;

	case 1:
		col = STP1_COL;
		step = &Page.step[1];
		break;

	case 2:
		col = STP2_COL;
		step = &Page.step[2];
		break;

	case 3:
		col = STP3_COL;
		step = &Page.step[3];
		break;

	case 4:
		col = STP4_COL;
		step = &Page.step[4];
		break;

	case 5:
		col = STP5_COL;
		step = &Page.step[5];
		break;

	case 6:
		col = STP6_COL;
		step = &Page.step[6];
		break;

	case 7:
		col = STP7_COL;
		step = &Step;
		break;

	default:
		return;
	}

	for( int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++ )
	{
		GoToCursor(col, id -1);
		if(step->position[id] & Action::INVALID_BIT_MASK)
			printf("----");
		else if(step->position[id] & Action::TORQUE_OFF_BIT_MASK)
			printf("????");
		else
			printf("%.4d", step->position[id]);
	}

	GoToCursor(col, PAUSE_ROW);
	printf("%4.3d", step->pause);

	GoToCursor(col, SPEED_ROW);
	printf("%4.3d", step->time);
	
	GoToCursor( old_col, old_row );
}

void DrawStepLine(bool erase)
{
	int old_col = Col;
	int old_row = Row;
	int col;

	switch(Page.header.stepnum)
	{
	case 0:
		col = STP0_COL;
		break;

	case 1:
		col = STP1_COL;
		break;

	case 2:
		col = STP2_COL;
		break;

	case 3:
		col = STP3_COL;
		break;

	case 4:
		col = STP4_COL;
		break;

	case 5:
		col = STP5_COL;
		break;

	case 6:
		col = STP6_COL;
		break;

	case 7:
		col = CWSLOPE_COL;
		break;

	default:
		return;
	}
	col--;

	for( int id=Action::ID_R_SHOULDER_PITCH; id<(Action::NUMBER_OF_JOINTS + 2); id++ )
	{
		GoToCursor(col, id - 1);
		if(erase == true)
			printf( " " );
		else
			printf( "|" );
	}

	GoToCursor(old_col, old_row);
}

void DrawName()
{
	int old_col = Col;
	int old_row = Row;

	GoToCursor(NAME_COL, NAME_ROW);
	printf( "                " );
	GoToCursor(NAME_COL, NAME_ROW);

	for(int i=0; i<Action::MAXNUM_NAME; i++)
		printf("%c", (char)Page.header.name[i]);
	
	GoToCursor( old_col, old_row );
}

void ClearCmd()
{
	PrintCmd("");
}

void PrintCmd(const char *message)
{
	int len = strlen(message);
	GoToCursor(0, CMD_ROW);

	printf( "] %s", message);
	for(int i=0; i<(SCREEN_COL - (len + 2)); i++)
		printf(" ");

	GoToCursor(len + 2, CMD_ROW);
}

int GetValue()
{
	int col;
	int row;
	if(bBeginCommandMode == true)
	{
		col = Old_Col;
		row = Old_Row;
	}
	else
	{
		col = Col;
		row = Row;
	}

	if( col == STP7_COL )
	{
		if( row == PAUSE_ROW )
			return Step.pause;
		else if( row == SPEED_ROW )
			return Step.time;
		else
			return Step.position[row + 1];
	}
	else if( col <= STP6_COL )
	{
		int i;
		switch(col)
		{
		case STP0_COL:
			i = 0;
			break;

		case STP1_COL:
			i = 1;
			break;

		case STP2_COL:
			i = 2;
			break;

		case STP3_COL:
			i = 3;
			break;

		case STP4_COL:
			i = 4;
			break;

		case STP5_COL:
			i = 5;
			break;

		case STP6_COL:
			i = 6;
			break;
		}

		if( row == PAUSE_ROW )
			return Page.step[i].pause;
		else if( row == SPEED_ROW )
			return Page.step[i].time;
		else
			return Page.step[i].position[row + 1];
	}
	else if(col == CWSLOPE_COL)
		return (Page.header.slope[row + 1] >> 4);
	else if(col == CCWSLOPE_COL)
		return (Page.header.slope[row + 1] & 0x0f);
	else if(col == PAGEPARAM_COL)
	{
		if(row == PLAYCOUNT_ROW)
			return Page.header.repeat;
		else if(row == STEPNUM_ROW)
			return Page.header.stepnum;
		else if(row == PLAYSPEED_ROW)
			return Page.header.speed;
		else if(row == ACCEL_ROW)
			return Page.header.accel;
		else if(row == NEXT_ROW)
			return Page.header.next;
		else if(row == EXIT_ROW)
			return Page.header.exit;
	}

	return -1;
}

void BeginCommandMode()
{
	Old_Col = Col;
	Old_Row = Row;
	ClearCmd();
	GoToCursor(CMD_COL, CMD_ROW);
	bBeginCommandMode = true;
}

void EndCommandMode()
{
	GoToCursor(Old_Col, Old_Row);
	bBeginCommandMode = false;
}

void HelpCmd()
{
	system("clear");
	printf(" exit               Exits the program.\n");
	printf(" re                 Refreshes the screen.\n");
	printf(" b                  Move to previous page.\n");
	printf(" n                  Move to next page.\n");
	printf(" page [index]       Move to page [index].\n");
	printf(" list               View list of pages.\n");
	printf(" new                Clears data of current page and initializes page.\n");
	printf(" copy [index]       Copy data from page [index].\n");
	printf(" set [value]        Sets value on cursor [value].\n");
	printf(" save               Saves changes.\n");
	printf(" play               Motion playback of current page.\n");
    printf(" g [index]          Motion playback of STP[index].\n");
	printf(" name               Name for current page or changes the name of current page.\n");
	printf(" time               Change time base playing.\n");
	printf(" speed              Change speed base playing.\n");
	printf(" w [index]          Overwrites data from STP[index] with STP7.\n");
	printf(" i                  Inserts data from STP7 to STP0. \n" \
	       "                    Moves data from STP[x] to STP[x+1].\n");
	printf(" i [index]          Inserts data from STP7 to STP[index]. \n" \
	       "                    Moves data from STP[index] to STP[index+1].\n");
	printf(" m [index] [index2] Moves data from [index] to [index2] step.\n");
	printf(" d [index]          Deletes data from STP[index]. \n"
	       "                    Pushes data from STP[index] to STP[index-1].\n");
	printf(" on/off             Turn On/Off torque from ALL actuators.\n");
	printf(" on/off [index1] [index2] ...  \n"
	       "                    turns On/Off torque from ID[index1] ID[index2]...\n");
	printf("\n");
	printf("       Copyright ROBOTIS CO.,LTD.\n");
	printf("\n");
	printf(" Press any key to continue...");
	_getch();

	DrawPage();
}

void NextCmd()
{
	PageCmd(indexPage + 1);
}

void PrevCmd()
{
	PageCmd(indexPage - 1);
}

void PageCmd(int index)
{
	if(AskSave() == true)
		return;

	if(index > 0 && index < Action::MAXNUM_PAGE)
	{
		indexPage = index;
		Action::GetInstance()->LoadPage(indexPage, &Page);

		Col = STP7_COL;
		Row = ID_1_ROW;
		DrawPage();
	}
	else
		PrintCmd("Invalid page index");

	bEdited = false;
}

void TimeCmd()
{
	Page.header.schedule = Action::TIME_BASE_SCHEDULE;
	bEdited = true;
	DrawPage();
}

void SpeedCmd()
{
	Page.header.schedule = Action::SPEED_BASE_SCHEDULE;
	bEdited = true;
	DrawPage();
}

void ListCmd()
{
	int old_col = Col;
	int old_row = Row;
	int index = 0;
	
	while(1)
	{
		system("clear");
		for(int i=0; i<22; i++)
		{
			for(int j=0; j<4; j++)
			{
				int k = (index * 88) + (j*22 + i);				
				Action::PAGE page;
				if(Action::GetInstance()->LoadPage(k, &page) == true)
				{
					printf(" |%.3d.", k);
					for(int n=0; n<Action::MAXNUM_NAME; n++)
					{
						if((char)page.header.name[n] >= ' ' && (char)page.header.name[n] <= '~')
							printf("%c", (char)page.header.name[n]);
						else
							printf(" ");
					}
				}
				else
				{
					printf(" |                ");
				}
			}
			printf("\n");
		}

		printf("\nAction Page List (%d/3) - Press key n(Next), b(Prev), q(Quit)", index + 1);
		while(1)
		{
			int ch = _getch();
			if(ch == 'n')
			{
				if(index < 2)
				{
					index++;
					break;
				}
			}
			else if(ch == 'b')
			{
				if(index > 0)
				{
					index--;
					break;
				}
			}
			else if(ch == 'q')
			{
				DrawPage();
				GoToCursor(old_col, old_row);
				return;
			}
		}
	}
}

void WriteStepCmd(int index)
{
	for(int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++)
	{
		if(Step.position[id] & Action::TORQUE_OFF_BIT_MASK)
			return;
	}

	if(index >= 0 && index < Action::MAXNUM_STEP)
	{
		Page.step[index] = Step;
		DrawStep(index);
		bEdited = true;
	}
	else
		PrintCmd("Invalid step index");
}

void DeleteStepCmd(int index)
{
	if(index >= 0 && index < Action::MAXNUM_STEP)
	{
		for(int i=index; i<Action::MAXNUM_STEP; i++)
		{
			if(i == (Action::MAXNUM_STEP - 1))
			{
				for(int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++)
					Page.step[i].position[id] = Action::INVALID_BIT_MASK;

				Page.step[i].pause = 0;
				Page.step[i].time = 0;
			}
			else
				Page.step[i] = Page.step[i + 1];
			
			DrawStep(i);
		}

		if(index < Page.header.stepnum)
		{
			if(Page.header.stepnum != 0)
			{
				DrawStepLine(true);
				Page.header.stepnum--;
				DrawStepLine(false);
			}

			GoToCursor(PAGEPARAM_COL, STEPNUM_ROW);
			printf( "%.3d", Page.header.stepnum );
		}

		bEdited = true;
	}
	else
		PrintCmd("Invalid step index");
}

void InsertStepCmd(int index)
{
	for(int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++)
	{
		// if(Step.position[id] & Action::TORQUE_OFF_BIT_MASK)
		// {			
		// 	PrintCmd("Exist invalid joint value");
		// 	return;
		// }
	}

	if(index >= 0 && index < Action::MAXNUM_STEP)
	{
		for(int i=Action::MAXNUM_STEP-1; i>index; i-- )
		{
			Page.step[i] = Page.step[i-1];
			DrawStep(i);
		}

		Page.step[index] = Step;
		DrawStep(index);

		if(index == 0 || index < Page.header.stepnum)
		{
			if(Page.header.stepnum != Action::MAXNUM_STEP)
			{
				DrawStepLine(true);
				Page.header.stepnum++;
				DrawStepLine(false);
			}

			GoToCursor(PAGEPARAM_COL, STEPNUM_ROW);
			printf( "%.3d", Page.header.stepnum );
		}

		bEdited = true;
	}
	else
		PrintCmd("Invalid step index");
}

void MoveStepCmd(int src, int dst)
{
	if(src < 0 || src >= Action::MAXNUM_STEP)
	{
		PrintCmd("Invalid step index");
		return;
	}

	if(dst < 0 || dst >= Action::MAXNUM_STEP)
	{
		PrintCmd("Invalid step index");
		return;
	}

	if(src == dst)
		return;

	Action::STEP step = Page.step[src];
	if(src < dst)
	{
		for(int i=src; i<dst; i++)
		{
			Page.step[i] = Page.step[i + 1];		
			DrawStep(i);
		}
	}
	else
	{
		for(int i=src; i>dst; i--)
		{
			Page.step[i] = Page.step[i - 1];		
			DrawStep(i);
		}
	}

	Page.step[dst] = step;
	DrawStep(dst);
	bEdited = true;
}

void CopyCmd(int index)
{
	if(index == indexPage)
		return;

	if(Action::GetInstance()->LoadPage(index, &Page) == true)
	{
		DrawPage();
		bEdited = true;
	}
	else
		PrintCmd("Invalid page index");
}

void NewCmd()
{
	Action::GetInstance()->ResetPage(&Page);
	DrawPage();
	bEdited = true;
}

void SaveCmd()
{
	if(bEdited == false)
		return;

	if(Action::GetInstance()->SavePage(indexPage, &Page) == true)
		bEdited = false;
}

void NameCmd()
{
	ClearCmd();
	GoToCursor(CMD_COL, CMD_ROW);
	printf("name: ");
	char name[80] = {0};
	fgets(name, 80, stdin);
	fflush(stdin);
	for(int i=0; i<=Action::MAXNUM_NAME; i++)
	{
		if (name[i] == '\n')
			name[i] = '\0';

		Page.header.name[i] = name[i];
	}
	DrawName();
	bEdited = true;
}

std::string get_joint_name(int id)
{
	switch(id)
	{
	case Action::ID_R_SHOULDER_PITCH:
		return "right_shoulder_pitch";
	case Action::ID_L_SHOULDER_PITCH:
		return "left_shoulder_pitch";
	case Action::ID_R_SHOULDER_ROLL:
		return "right_shoulder_roll";
	case Action::ID_L_SHOULDER_ROLL:
		return "left_shoulder_roll";
	case Action::ID_R_ELBOW:
		return "right_elbow";
	case Action::ID_L_ELBOW:
		return "left_elbow";
	case Action::ID_R_HIP_YAW:
		return "right_hip_yaw";
	case Action::ID_L_HIP_YAW:
		return "left_hip_yaw";
	case Action::ID_R_HIP_ROLL:
		return "right_hip_roll";
	case Action::ID_L_HIP_ROLL:
		return "left_hip_roll";
	case Action::ID_R_HIP_PITCH:
		return "right_hip_pitch";
	case Action::ID_L_HIP_PITCH:
		return "left_hip_pitch";
	case Action::ID_R_KNEE:
		return "right_knee";
	case Action::ID_L_KNEE:
		return "left_knee";
	case Action::ID_R_ANKLE_PITCH:
		return "right_ankle_pitch";
	case Action::ID_L_ANKLE_PITCH:
		return "left_ankle_pitch";
	case Action::ID_R_ANKLE_ROLL:
		return "right_ankle_roll";
	case Action::ID_L_ANKLE_ROLL:
		return "left_ankle_roll";
	case Action::ID_HEAD_PAN:
		return "neck_yaw";
	case Action::ID_HEAD_TILT:
		return "neck_pitch";
	case Action::ID_R_GRIPPER:
		return "right_gripper";
	case Action::ID_L_GRIPPER:
		return "left_gripper";
	default:
		return "unknown";
	}
}

double to_angle(double position)
{
	return (position - 2048.0) * (360.0 / 4096.0);
}

std::string get_action_name(int index)
{
	Action::PAGE page;
	Action::GetInstance()->LoadPage(index, &page);
	return std::string(reinterpret_cast<char*>(page.header.name));
}

// Write current page to json file
void WriteJson()
{
	nlohmann::json json_action;

	std::string action_name(reinterpret_cast<char*>(Page.header.name));

	json_action["name"] = action_name;
	json_action["next"] = get_action_name(Page.header.next);

	// int invalid_bit_mask_count = 0;
	for (int i = 0; i < Page.header.stepnum; i++)
	{
		nlohmann::json json_pose;
		nlohmann::json json_joint;
		for (int id=Action::ID_R_SHOULDER_PITCH; id<Action::NUMBER_OF_JOINTS; id++)
		{
			double position = 0.0;
			if(!(Page.step[i].position[id] & Action::INVALID_BIT_MASK))
			{
				position = to_angle(Page.step[i].position[id]);
			}

			json_joint[get_joint_name(id)] = position;
		}

		json_pose["joints"] = json_joint;
		json_pose["name"] = "step_" + std::to_string(i);

		double pause_time = Page.step[i].pause * 8.0 / 1000.0;
		json_pose["pause"] = pause_time;

		double time = Page.step[i].time * 8.0 / 1000.0;
		json_pose["time"] = time;
	
		json_pose["speed"] = 0.0;

		json_action["poses"].push_back(json_pose);
	}

	json_action["start_delay"] = 0.0;
	json_action["stop_delay"] = 0.0;
	json_action["time_based"] = true;

	std::string json_str = json_action.dump(4);

	std::string file_name = "../" + action_name + ".json";

	FILE *fp = fopen(file_name.c_str(), "w");
	if (fp == NULL)
	{
		PrintCmd("Failed to open file");
		return;
	}

	fprintf(fp, "%s", json_str.c_str());
	fclose(fp);
}
