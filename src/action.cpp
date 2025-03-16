#include <string.h>
#include <unistd.h>
#include <iostream>

#include "action.h"

Action* Action::m_UniqueInstance = NULL;

Action::Action()
{
	DEBUG_PRINT = true;
	m_ActionFile = 0;
	m_Playing = false;
    wUnitTimeCountAddition = 1;
}

Action *Action::GetInstance()
{
    if (m_UniqueInstance == NULL)
    {
        m_UniqueInstance = new Action();
    }

    return m_UniqueInstance;
}

Action::~Action()
{
	if(m_ActionFile != 0)
		fclose( m_ActionFile );
}

bool Action::VerifyChecksum( PAGE *pPage )
{
	unsigned char checksum = 0x00;
    unsigned char *pt = (unsigned char*)pPage;

    for(unsigned int i = 0; i < sizeof(PAGE); i++)
    {
        checksum += *pt;
        pt++;
    }

    if(checksum != 0xff)
        return false;

    return true;
}

void Action::SetChecksum( PAGE *pPage )
{
	unsigned char checksum = 0x00;
    unsigned char *pt = (unsigned char*)pPage;

    pPage->header.checksum = 0x00;

    for(unsigned int i=0; i<sizeof(PAGE); i++)
    {
        checksum += *pt;
        pt++;
    }

    pPage->header.checksum = (unsigned char)(0xff - checksum);
}

void Action::ResetPage(PAGE *pPage)
{
	unsigned char *pt = (unsigned char*)pPage;

    for(unsigned int i=0; i<sizeof(PAGE); i++)
    {
        *pt = 0x00;
        pt++;
    }

    pPage->header.schedule = TIME_BASE_SCHEDULE; // default time base
    pPage->header.repeat = 1;
    pPage->header.speed = 32;
    pPage->header.accel = 32;

	for(int i=0; i<NUMBER_OF_JOINTS; i++)
        pPage->header.slope[i] = 0x55;

    for(int i=0; i<MAXNUM_STEP; i++)
    {
        for(int j=0; j<31; j++)
            pPage->step[i].position[j] = INVALID_BIT_MASK;

        pPage->step[i].pause = 0;
        pPage->step[i].time = 0;
    }

    SetChecksum( pPage );
}

void Action::Initialize()
{
	m_Playing = false;
}

bool Action::LoadFile( char* filename )
{
	FILE *action = fopen(filename, "r+b" );
    if( action == 0 )
	{
		if(DEBUG_PRINT == true)
			printf("Can not open Action file! %s\n", filename);
        return false;
	}

    fseek( action, 0, SEEK_END );
    if( ftell(action) != (long)(sizeof(PAGE) * MAXNUM_PAGE) )
    {
		if(DEBUG_PRINT == true)
			printf("It's not an Action file!\n");
        fclose( action );
        return false;
    }

	if(m_ActionFile != 0)
		fclose( m_ActionFile );

	m_ActionFile = action;
	return true;
}

void Action::Stop()
{
	m_StopPlaying = true;
}

void Action::Brake()
{
	m_Playing = false;
}

bool Action::IsRunning()
{
	return m_Playing;
}

bool Action::IsRunning(int *iPage, int *iStep)
{
	if(iPage != 0)
		*iPage = m_IndexPlayingPage;

	if(iStep != 0)
		*iStep = m_PageStepCount - 1;

	return IsRunning();
}

bool Action::LoadPage(int index, PAGE *pPage)
{
	long position = (long)(sizeof(PAGE)*index);

    if( fseek( m_ActionFile, position, SEEK_SET ) != 0 )
        return false;

    if( fread( pPage, 1, sizeof(PAGE), m_ActionFile ) != sizeof(PAGE) )
        return false;

    if( VerifyChecksum( pPage ) == false )
        ResetPage( pPage );

    return true;
}

bool Action::SavePage(int index, PAGE *pPage)
{
	long position = (long)(sizeof(PAGE)*index);

	if( VerifyChecksum(pPage) == false ) SetChecksum(pPage);

    if( fseek( m_ActionFile, position, SEEK_SET ) != 0 ) return false;

    if( fwrite( pPage, 1, sizeof(PAGE), m_ActionFile ) != sizeof(PAGE) ) return false;

	return true;
}
