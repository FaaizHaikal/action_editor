#ifndef ACTION_H_
#define ACTION_H_

#include <stdio.h>
#include <string>

class Action {
public:
  enum
	{
		MAXNUM_PAGE = 256,
		MAXNUM_STEP = 7,
		MAXNUM_NAME = 13
	};

  enum
  {
    ID_R_SHOULDER_PITCH     = 1,
    ID_L_SHOULDER_PITCH     = 2,
    ID_R_SHOULDER_ROLL      = 3,
    ID_L_SHOULDER_ROLL      = 4,
    ID_R_ELBOW              = 5,
    ID_L_ELBOW              = 6,
    ID_R_HIP_YAW            = 7,
    ID_L_HIP_YAW            = 8,
    ID_R_HIP_ROLL           = 9,
    ID_L_HIP_ROLL           = 10,
    ID_R_HIP_PITCH          = 11,
    ID_L_HIP_PITCH          = 12,
    ID_R_KNEE               = 13,
    ID_L_KNEE               = 14,
    ID_R_ANKLE_PITCH        = 15,
    ID_L_ANKLE_PITCH        = 16,
    ID_R_ANKLE_ROLL         = 17,
    ID_L_ANKLE_ROLL         = 18,
    ID_HEAD_PAN             = 19,
    ID_HEAD_TILT            = 20,
    ID_R_GRIPPER			      = 21,
    ID_L_GRIPPER			      = 22,
    NUMBER_OF_JOINTS
  };

	enum
	{
		SPEED_BASE_SCHEDULE = 0,
		TIME_BASE_SCHEDULE = 0x0a
	};
	enum
	{
		INVALID_BIT_MASK	= 0x4000,
		TORQUE_OFF_BIT_MASK	= 0x2000
	};

  typedef struct // Header Structure (total 64unsigned char)
	{
		unsigned char name[MAXNUM_NAME+1]; // Name             0~13
		unsigned char reserved1;        // Reserved1        14
		unsigned char repeat;           // Repeat count     15
		unsigned char schedule;         // schedule         16
		unsigned char reserved2[3];     // reserved2        17~19
		unsigned char stepnum;          // Number of step   20
		unsigned char reserved3;        // reserved3        21
		unsigned char speed;            // Speed            22
		unsigned char reserved4;        // reserved4        23
		unsigned char accel;            // Acceleration time 24
		unsigned char next;             // Link to next     25
		unsigned char exit;             // Link to exit     26
		unsigned char reserved5[4];     // reserved5        27~30
		unsigned char checksum;         // checksum         31
		unsigned char slope[31];        // CW/CCW compliance slope  32~62
		unsigned char reserved6;        // reserved6        63
	} PAGEHEADER;
	typedef struct // Step Structure (total 64unsigned char)
	{
		unsigned short position[31];    // Joint position   0~61
		unsigned char pause;            // Pause time       62
		unsigned char time;             // Time             63
	} STEP;
	typedef struct // Page Structure (total 512unsigned char)
	{
		PAGEHEADER header;          // Page header  0~64
		STEP step[MAXNUM_STEP];		// Page step    65~511
	} PAGE;

  bool DEBUG_PRINT;

	Action();
	~Action();

	static Action* GetInstance();

	void Initialize();
	bool LoadFile(char* filename);
	void Stop();
	void Brake();
	bool IsRunning();
	bool IsRunning(int *iPage, int *iStep);
	bool LoadPage(int index, PAGE *pPage);
	bool SavePage(int index, PAGE *pPage);
	void ResetPage(PAGE *pPage);
	int wUnitTimeCountAddition;

private:
	static Action* m_UniqueInstance;
	FILE* m_ActionFile;
	PAGE m_PlayPage;
	PAGE m_NextPlayPage;
	STEP m_CurrentStep;

	int m_IndexPlayingPage;
	bool m_FirstDrivingStart;
	int m_PageStepCount;
	bool m_Playing;
	bool m_StopPlaying;
	bool m_PlayingFinished;

	bool VerifyChecksum( PAGE *pPage );
	void SetChecksum( PAGE *pPage );
};

#endif // ACTION_H_
