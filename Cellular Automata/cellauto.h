#ifndef CELLAUTO_H_INCLUDED
#define CELLAUTO_H_INCLUDED

#include <array>
#include <vector>
#include <algorithm>
#include <utility>
#include <Windows.h>
#include <cstdio>
#include <iostream>
HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
struct StandardCell {
	const static unsigned short x = 100;
	const static unsigned short y = 69;
};
volatile unsigned numofalivecells = 0;
template<typename e>
struct CellularAutomata {
	CellularAutomata() = delete;
	static void old_render_scene(const std::array<std::array<std::pair<bool, unsigned short>, e::x>, e::y>& input, std::array<std::array<std::pair<bool, unsigned short>, e::x>, e::y>& oldinput, char&& OutChar, short&& offx, short&& offy) {
		unsigned short LastColor = 0xF;
		for (short y = 0; y < e::y; y++) {
			bool change = false;
			for (short x = 0; x < e::x; x++) {
				if (std::get<0>(input[y][x]) != std::get<0>(oldinput[y][x])) {
					if (!change) {
						change = true;
						SetConsoleCursorPosition(hstdout, { static_cast<short>(offx + x),static_cast<short>(offy + y) });
					}
					if (std::get<0>(input[y][x])) {
						if (LastColor != std::get<1>(input[y][x])) {
							SetConsoleTextAttribute(hstdout, std::get<1>(input[y][x]));
						}
						std::cout << OutChar;
					}
					else {
						std::cout << ' ';
					}

				}
				else {
					change = false;
				}
			}
		}
		oldinput = input;
		/*Efficient Method for Rendering
		Only renders blocks that have change
		Only uses SetConsoleCursorPosition & SetConsoleTextAttribute when it absolutely has to.
		Takes advantage of cout automatic increment to the cursor x position
		*/
	}
	static void render_scene(const std::array<std::array<std::pair<bool, unsigned short>, e::x>,e::y>& input,char&& OutChar) {
		//Renders Scenes starting from the top left corner
		//Can print only one type of character but at different colors
		unsigned short LastColor = std::get<1>(input[0][0]);
		SetConsoleTextAttribute(hstdout, LastColor);
		SetConsoleCursorPosition(hstdout, { 0,0 });
		//Where we store characters to be printed out
		std::string buffer;
		for (auto& row : input) {
			if (!buffer.empty()) {
				buffer += '\n';
			}
			for (auto& block : row) {
				if (block.first) {
					if (block.second == LastColor) {
						buffer += OutChar;
					}
					else {
						fwrite(buffer.c_str(), 1, buffer.size(), stdout);
						buffer.clear();
						LastColor = block.second;
						SetConsoleTextAttribute(hstdout, LastColor);
						buffer += OutChar;
					}
				}
				else {
					buffer += ' ';
				}
			}
		}
		fwrite(buffer.c_str(), 1, buffer.size(), stdout);
	}
	static void apply_rule_gol(std::array<std::array<std::pair<bool, unsigned short>, e::x>, e::y>& input, const std::array<std::array<std::pair<bool, unsigned short>, e::x>, e::y>& oldinput) {
	#pragma omp parallel
		{
			int changecell = 0; //How many net count of cells removed or added in during a frame
	#pragma omp for
			for (short y = 0; y < e::y; y++) {
				for (short x = 0; x < e::x; x++) {
					int neighbors = 0;
					//Gets the total amount of neighbors near our square
					//Can handle wrapping around the inputs
					std::vector<unsigned short> modecalc;
					//A vector of the neighbors colors to calculate mean
					for (int offy = -1; offy < 2; offy++) {
						for (int offx = -1; offx < 2; offx++) {
							if ((offx != 0 || offy != 0) && std::get<0>(oldinput[mod(y + offy, e::y)][mod(x + offx, e::x)])) {
								neighbors++;
								modecalc.push_back(std::get<1>(oldinput[mod(y + offy, e::y)][mod(x + offx, e::x)]));
								if (neighbors > 3) {
									goto Skip;
								}
							}

						}
					}
				Skip:;
					//Once the neighbors are found we apply the condition checks based on whether it is dead or alive
					if (std::get<0>(oldinput[y][x])) {
						if (neighbors < 2 || neighbors > 3) {
							//If it dies it retains the original color in the dead state
							input[y][x] = std::make_pair(false, std::get<1>(input[y][x]));
								changecell--;
						}
					}
					else {
						if (neighbors == 3) {
							//If it is alive we do a simple and quick check to see if all the values are the same and if so simply return its own value
							if ((modecalc[0] + modecalc[1] + modecalc[2]) / 3 == modecalc[0]) {
								input[y][x] = std::make_pair(true, modecalc[0]);
							}
							else {
								//Otherwise we return the mode
								std::array<unsigned short, 15> histogram{ 0 };
								for (auto &histocell : modecalc) {
									++histogram[histocell - 1];
								}
								unsigned short ModeCurr = 0; //Stores the current mode
							
								for (unsigned short i = 1; i < 15; i++) {
									if (histogram[i] > histogram[ModeCurr]) {
										ModeCurr = i;
									}
								}
								input[y][x] = std::make_pair(true, ModeCurr + 1);
							}
							changecell++;
						}
					}
				}
			}
	#pragma omp critical 
			{
				numofalivecells += changecell;
			}
		}
	}
private:
	static int mod(int a, int b) {
		//Modulus as % represents remainder
		int r = a % b;
		return r < 0 ? r + b : r;
	}
	
};

#endif // CELLAUTO_H_INCLUDED
