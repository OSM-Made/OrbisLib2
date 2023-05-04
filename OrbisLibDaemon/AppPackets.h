#pragma once

struct AppInfoPacket
{
	char TitleId[10];
	char ContentId[100];
	char TitleName[200];
	char MetaDataPath[100];
	char LastAccessTime[100];
	int Visible;
	int SortPriority;
	int DispLocation;
	bool CanRemove;
	char Category[10];
	int ContentSize;
	char InstallDate[100];
	char UICategory[10];
};

enum AppState
{
	STATE_NOT_RUNNING,
	STATE_RUNNING,
	STATE_SUSPENDED,
};