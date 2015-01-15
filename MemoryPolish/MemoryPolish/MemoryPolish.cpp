#define _CRT_SECURE_NO_WARNINGS		// sscanf
#include "RMenu.h"
#include "RDatabase.h"
#include "RObject.h"
#include <tchar.h>
#include <iostream>
#include <string>
#include "Rtchar.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "RString.h"
#include <algorithm>
using namespace std;
using namespace rl;

const tstring DATABASE_FILENAME = _T("database.dat");
RDatabase g_db;

int Continue(void*)
{
	return RMenu::BREAK;
}

int Back(void*)
{
	return RMenu::BREAK;
}

int New(void*)
{
	RMenu menu;
	menu.Add(_T("(C)ontinue"), Continue, 0);
	menu.AddHotkey('c', Continue, 0);
	menu.AddHotkey('C', Continue, 0);
	menu.Add(_T("(B)ack"), Back, 0);
	menu.AddHotkey('b', Back, 0);
	menu.AddHotkey('B', Back, 0);
	do
	{
		tstring Q;
		tcout << _T("Q: ");
		getline(tcin, Q);
		tcout << _T("A: ");
		tstring A;
		getline(tcin, A);
		shared_ptr<RObject> obj = make_shared<RObject>(Q, A);
		obj->SetAttribute(_T("ts"), Sprintf(_T("%d"), time(0) + 600));
		obj->SetAttribute(_T("fresh"), _T("600"));	// 10 mins
		g_db.Set(obj);

		int sel = menu.Run();
		switch (sel)
		{
		case 0: case 'c': case 'C':
			continue;
		case 1: case 'b': case 'B':
			return RMenu::CONTINUE;
		}
	}
	while (true);
}

bool FindTimeout(shared_ptr<RObject> obj)
{
	int ts;
	tsscanf(obj->GetAttribute(_T("ts"))->GetValue().c_str(), _T("%d"), &ts);
	return (time(0) >= ts);
}

int Review(void*)
{
	auto filter = g_db.Filter(FindTimeout);

	// randomize the order in filter (and store into the variable db)
	vector<pair<int,int> > order(filter->Size());
	srand(time(0));
	for (size_t i = 0; i < order.size(); i++)
	{
		order[i].first = rand();
		order[i].second = i;
	}
	sort(order.begin(), order.end(), [](pair<int,int>& i1, pair<int,int>& i2){return i1.first < i2.first;});
	vector<shared_ptr<RObject> > db(order.size());
	int i = 0;
	auto obj = filter->First();
	while (!obj->IsEmpty())
	{
		db[order[i++].second] = obj;
		obj = filter->Next();
	}

	tstring info;
	//shared_ptr<RObject> 
	RMenu menu;
	menu.Add(_T("(C)heck"), Continue, 0);
	menu.AddHotkey('c', Continue, 0);
	menu.AddHotkey('C', Continue, 0);
	menu.Add(_T("(S)kip"), Continue, 0);
	menu.AddHotkey('s', Continue, 0);
	menu.AddHotkey('S', Continue, 0);
	menu.Add(_T("(B)ack"), Back, 0);
	menu.AddHotkey('b', Back, 0);
	menu.AddHotkey('B', Back, 0);
	menu.SetInfo( [](void* str)->tstring{ return *(tstring*)str; }, &info );
	for (int i = 0; i < db.size(); i++)
	{
		info = _T("Q: ") + db[i]->GetName();
		int sel = menu.Run();
		switch (sel)
		{
		case 0: case 'c': case 'C':
			{
				tcout << _T("A: ") + db[i]->GetValue() << endl;
				tcout << _T("Score (0-100) You Get: ");
				int score;
				tcin >> score;
				int fresh;
				tsscanf(db[i]->GetAttribute(_T("fresh"))->GetValue().c_str(), _T("%d"), &fresh);
				fresh = ((10 + score) * fresh ) / 20;
				int ts = time(0) + fresh;
				db[i]->SetAttribute(_T("ts"), Sprintf(_T("%d"), ts));
				db[i]->SetAttribute(_T("fresh"), Sprintf(_T("%d"), fresh));
			}
			break;
		case 1: case 's': case 'S':
			break;
		case 2: case 'b': case 'B':
			return RMenu::CONTINUE;
		}
	}
	return RMenu::CONTINUE;
}

int Load(void*)
{
	g_db.Load(DATABASE_FILENAME);
	return RMenu::CONTINUE;
}

int Save(void*)
{
	g_db.Save(DATABASE_FILENAME);
	return RMenu::CONTINUE;
}

int List(void*)
{
	auto rec = g_db.First();
	while (!rec->IsEmpty())
	{
		tcout << rec->GetName() << endl;
		rec = g_db.Next();
	}
	system("pause");
	return RMenu::CONTINUE;
}

int Exit(void* arg)
{
	g_db.Save(DATABASE_FILENAME);
	exit((int)arg);
	return RMenu::BREAK;	// never comes here
}

void SaveAndExit()
{
	Save(0);
	Exit(0);
}

tstring Info(void*)
{
	return Sprintf(_T("Total Record: %d"), g_db.Size());
}

int main()
{
	atexit(SaveAndExit);

	g_db.Load(DATABASE_FILENAME);

	RMenu menu;
	menu.SetInfo(Info, 0);
	menu.Add(_T("(N)ew"), New, 0);
	menu.AddHotkey('n', New, 0);
	menu.AddHotkey('N', New, 0);
	menu.Add(_T("(T)est"), Review, 0);
	menu.AddHotkey('t', Review, 0);
	menu.AddHotkey('T', Review, 0);
	menu.Add(_T("(R)eload"), Load, 0);
	menu.AddHotkey('r', Load, 0);
	menu.AddHotkey('R', Load, 0);
	menu.Add(_T("(S)ave"), Save, 0);
	menu.AddHotkey('s', Save, 0);
	menu.AddHotkey('S', Save, 0);
	menu.Add(_T("(L)ist"), List, 0);
	menu.AddHotkey('l', List, 0);
	menu.AddHotkey('L', List, 0);
	menu.Add(_T("(E)xit"), Exit, 0);
	menu.AddHotkey('e', Exit, 0);
	menu.AddHotkey('E', Exit, 0);
	menu.Run();
	return 0;
}