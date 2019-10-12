//Simple Cell Automata Setup
//Can support cells that are either alive or dead. With custom colors
//By default comes with game of life
//
//By Nicholas Dundas
// 4/6/2018
#include <chrono>
#include <random>
#include <string>
#include <fstream>
#include "cellauto.h"
void SetWindow(SHORT Width, SHORT Height)
{
	_COORD coord;
	coord.X = Width;
	coord.Y = Height;

	_SMALL_RECT Rect;
	Rect.Top = 0;
	Rect.Left = 0;
	Rect.Bottom = Height - 1;
	Rect.Right = Width - 1;

	SetConsoleScreenBufferSize(hstdout, coord);            // Set Buffer Size
	SetConsoleWindowInfo(hstdout, TRUE, &Rect);            // Set Window Size
}
using namespace std;
int main() {
	SetConsoleTitleW(L"Conway's Game of Life");
	SetConsoleTextAttribute(hstdout, 0x0F);
	SetWindow(102, 82);
	long long deltatime; //Time in between frames (in ms)
						 //Store milliseconds passed
	unsigned long long generations = 0; //How many generations have passed
	array<array<pair<bool, unsigned short>, 100>, 69> input, oldinput; //For storing the input from a file; oldinput is used as comparison to the previous input
	oldinput[0].fill(make_pair(false, 0));
	oldinput.fill(oldinput[0]);
	//We set this input false as it is used as compairson when rendering
RepeatCol:
	cout << "Type bw for black and white or col for color.\n?";
	string strin; //Standard user input
	cin >> strin;
	for (auto &x : strin) {
		if (x <= 'Z' && x >= 'A') {
			x = x + 32;
		}
	}
	bool docolor = true;
	if (strin == "bw") {
		docolor = false;
	}
	else if (strin == "col") {
		docolor = true;
	}
	else {
		goto RepeatCol;
	}
Repeat:
	cout << "Type file to open a file or rand for random.\n?";
	cin >> strin;
	for (auto &x : strin) {
		if (x <= 'Z' && x >= 'A') {
			x = x + 32;
		}
	}
	random_device rd;
	mt19937 gen{ rd() };
	bernoulli_distribution dis(.33);
	bernoulli_distribution coldis(.65);
	unsigned char tempcol;
	if (docolor) {
		tempcol = gen() % (0xE) + 1;
	}
	else {
		tempcol = 0x0F;
	}
	if (strin == "rand") {
		fstream fout("prev.cell", fstream::out);
		//Simple random number generator
		for (auto &Row : input) {
			if (coldis(gen)) {
				if (docolor) {
					tempcol = gen() % (0x0E) + 1;
				}
				else {
					tempcol = 0x0F;
				}
			}
			for (auto &block : Row) {
				block = make_pair(dis(gen), tempcol);
				if (std::get<0>(block)) {
					fout << '1';
					numofalivecells++;
				}
				else {
					fout << '0';
				}
			}
			fout << '\n';
		}
		fout.close();
	}
	else if (strin == "file") {
	FileErr:
		cout << "Enter file name...\n?";
		cin >> strin;
		ifstream File(strin);
		if (!File.is_open()) {
			cout << strin << " file not found!\n";
			goto FileErr;
		}
		char c = '0';
		for (auto &Row : input) {
			if (docolor) {
				tempcol = gen() % (0x0E) + 1;
			}
			else {
				tempcol = 0x0F;
			}
			for (auto &block : Row) {
			GetNextChar:
				if (File.get(c)) {
					if (c == '1') {
						block = make_pair(true, tempcol);
						numofalivecells++;
					}
					else if (c == '0') {
						block = make_pair(false, tempcol);
					}
					else {
						goto GetNextChar;
					}
				}
				else {
					cout << "File Read Err...\n";
					goto FileErr;
				}
			}
		}
	}
	else {
		goto Repeat;
	}
RepeatGen:
	cout << "How many generations, type -1 for infinite.\n?";
	unsigned long long GenMax;
	cin >> strin;
	try {
		GenMax = stoull(strin);
	}
	catch (...) {
		cout << strin << " is not a number!\n";
		goto RepeatGen;
	}
	cout << "Minimum thread wait time.\n?";
	unsigned long temp;
	cin >> strin;
	try {
		temp = stoul(strin);
	}
	catch (...) {
		cout << strin << " is not a number!\n";
		goto RepeatGen;
	}
	//Filling input with random squares
	//Used to set the old input to false as it is set to undefined and as rendering uses it to compare
	const unsigned long ThreadMin = temp;
	unsigned oldalivecells = 0;
	for (unsigned long long x = 0; x != GenMax; x++) {
		chrono::time_point<chrono::high_resolution_clock> beginFrame = chrono::high_resolution_clock::now();
		CellularAutomata <StandardCell>::render_scene(input, 219);
		oldalivecells = numofalivecells;
		CellularAutomata<StandardCell>::apply_rule_gol(input, oldinput);
		oldinput = input;
		chrono::time_point<chrono::high_resolution_clock> endFrame = chrono::high_resolution_clock::now();
		deltatime = chrono::duration_cast<chrono::milliseconds>(endFrame - beginFrame).count();
		if (ThreadMin > deltatime) { Sleep(ThreadMin - deltatime);
}
		generations++;
		SetConsoleCursorPosition(hstdout, { 0,static_cast<short>(input.size()) });
		SetConsoleTextAttribute(hstdout, 0x0F);
		cout << "Time between frames in ms : " << ((ThreadMin > deltatime) ? ThreadMin : deltatime) << "            \nGeneration Number : " << generations << " | Cells : " << numofalivecells << "                ";
	
	}
	SetConsoleCursorPosition(hstdout, { 0,static_cast<SHORT>(input.size()) });
	cout << "\n\nFinished, press any key to exit...";
	cin.ignore();
	cin.get();
	return 0;
}

