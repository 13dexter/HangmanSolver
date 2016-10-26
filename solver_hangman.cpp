#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <ctime>
#include <vector>
#include <new>
using namespace std;

//Stores the dictionary without duplicates.
vector<string> all_words;

//Stores the dictionary.
vector<string> all_words_with_duplicates;

//Stores the "first guess" order of letters (for each length).
vector<vector<char> > finalTable;

//Stores a list of positions of strings (all_words) that are of length i, at a position j, have a charachter k.
vector<vector<vector<vector<int> > > > list;

//Stores the position of the first charater guessed.
int firstCharPos;

//Stores the max word length in the dictionary.
int maxWordLength;
ofstream printSteps;


//Returns true if the word contains any alphabets marked by list.
bool wordContainsLetters(string word, vector<int> list){
	int i;
	int l = word.size();
	for(i = 0; i < l; i++){
		if(list[word[i]-'a'] != 0) return true;
	}
	return false;
}


//Creates the vector finalTable (first guess order of letters).
void createFinalTable(){
	string word;
	finalTable.resize(maxWordLength+1);

	//Storing the final count of letters in the words in the entire dictionary.
	vector<vector<int> > alphaList, finalTableAsInt;
	alphaList.resize(maxWordLength+1);
	finalTableAsInt.resize(maxWordLength+1);

	int i, j; //iterator
	for(i = 0; i <= maxWordLength; i++){
		alphaList[i].resize(26, 0);
		finalTableAsInt[i].resize(26, 0);
	}
	
	//Storing letters that have already occurred in a word.
	vector<int> currentWord;

	//To switch rows in the dictionary vector alternatively reading/writing.
	int switchFiles = 0;
	int switchComplement = 1;

	vector<vector<string> > dictionary;
	dictionary.resize(2);

	dictionary[0] = all_words;

	//To terminate when all words have been processed.
	bool process = true;

	while(process){
		process = false;
		int len = dictionary[switchFiles].size();

		for(int j = 0; j < len; j++){
			currentWord.clear();
			currentWord.resize(26, 0);

			word = dictionary[switchFiles][j];
			int l = word.size();

			if(wordContainsLetters(word, finalTableAsInt[l])) continue;

			dictionary[switchComplement].push_back(word);
			process = true;

			for(i = 0; i < l; i++){
				int ch = word[i]-'a';
				if(currentWord[ch] != 0) continue;
				alphaList[l][ch]++;
				currentWord[ch]++;
			}
		}

		//Finds the best next guess for each word length.
		for(i = 1; i <= maxWordLength; i++){
			int max = 0, maxVal = alphaList[i][0];
			alphaList[i][0] = 0;
			for(j = 1; j < 26; j++){
				if(alphaList[i][j] > maxVal){
					max = j;
					maxVal = alphaList[i][max];
				}
				alphaList[i][j] = 0;
			}
			if(maxVal == 0) continue;
			finalTable[i].push_back((char)('a'+max));
			finalTableAsInt[i][max]++;
		}

		dictionary[switchFiles].clear();
		switchFiles = (switchFiles+1)%2;
		switchComplement = (switchComplement+1)%2;
	}
}




/*
Returns number of positions newly filled by guessed character.
Alters the answer being printed to new state.
Alters the guess vector according to the new guesses.
*/
int fillUpWordAccToGuess(char guess, string wordToGuess, char *answer, vector<int> &guessed){
	int l = wordToGuess.size();
	int numOfGuesses = 0;
	for(int i = 0; i < l; i++){
		if(guess == wordToGuess[i]){
			answer[i] = guess;
			guessed[i] = 1;
			numOfGuesses++;
			firstCharPos = i;
		}
	}
	return numOfGuesses;
}


/*
Creates all_words and all_words_with_duplicates lists from the dictionary given.
Also creates the list vector for fast access.
*/
void makeVector(){
	string word, prevWord = "";
	//to calculate the max word length and store values into all_word vectors
	while(cin >> word){
		all_words_with_duplicates.push_back(word);
		if(word == prevWord) continue;

		prevWord = word;
		all_words.push_back(word);
		int l = word.size();
		if(maxWordLength < l) maxWordLength = l;
	}

	list.resize(maxWordLength+1);
	int i, j;
	for(i = 1; i <= maxWordLength; i++){
		list[i].resize(i);
		for(int j = 0; j < i; j++){
			list[i][j].resize(26);
		}
	}

	int l = all_words.size();
	for(i = 0; i < l; i++){
		string str_word = all_words[i];
		int len = str_word.size();
		for(j = 0; j < len; j++){
			list[len][j][str_word[j]-'a'].push_back(i);
		}
	}
}



//Returns the string list asked (by specifying length, postion known and character at that position).
vector<string> createDictionaryCopy(int l, int pos, int ch){
	vector<string> copyListAsked;
	int len = list[l][pos][ch].size();
	for(int i = 0; i < len; i++){
		copyListAsked.push_back(all_words[list[l][pos][ch][i]]);
	}
	return copyListAsked;
}



//Returns true if word being tested matches already guessed pattern and does not contain misses.
bool wordMatches(string word, string guessWord, vector<int> &guessed, vector<int> &misses){
	int l = word.size();
	for(int i = 0 ; i < l; i++){
		if(guessed[i] == 1){
			if(word[i] != guessWord[i]) return false;
		}
		else if(misses[word[i]-'a'] == 1) return false;
	}
	return true;
}


/*
Solves hangman for each word.
Uses finalTable already generated and conditional probablity to make further guesses.
*/
int solver(string guessWord){
	printSteps <<"Word to guess : "<<guessWord<<endl;

	firstCharPos = -1;

	//Used to generate further guesses after first guess.
	vector<vector<string> > dictionary;
	dictionary.resize(2);

	//Length of word to guess.
	int l = guessWord.size();

	//Stores the state of the word being guessed.
	char *answer = (char*)malloc(l+1);

	//Stores the characters that were missed to print.
	string missPrint;

	for(int i = 0; i < l; i++){
		answer[i] = '_';
	}
	answer[l] = '\0';

	//Stores the alphabets hit and missed.
	vector<int> misses, hits; 
	misses.resize(26, 0);
	hits.resize(26, 0);

	//Stores the positions that have been guessed.
	vector<int> guessesPos;
	guessesPos.resize(l, 0);
	int guessResult;

	//Stores total guessed and total missed to terminate / end word processing when needed.
	int totalGuessed = 0, totalMisses = 0;

	int i; //iterator

	int tableLength = finalTable[l].size();

	//Guesses the first letter in the unknown word.
	for(i = 0; i < tableLength && totalMisses < 6; i++){
		guessResult = fillUpWordAccToGuess(finalTable[l][i], guessWord, answer, guessesPos);
		printSteps <<"Guess : "<<finalTable[l][i]<<endl;

		if(guessResult){
			totalGuessed += guessResult;
			hits[finalTable[l][i]-'a']++;
			
			printSteps <<answer<<endl;
			printSteps <<"Misses : "<<missPrint<<endl;
			printSteps <<"-------------------------------"<<endl;
			
			break;
		}
		missPrint += finalTable[l][i];
		totalMisses++;
		misses[finalTable[l][i]-'a']++;

		printSteps <<answer<<endl;
		printSteps <<"Misses : "<<missPrint<<endl;
		printSteps <<"-------------------------------"<<endl;
	}
	
	if(totalMisses < 6) dictionary[0] = createDictionaryCopy(l, firstCharPos, guessWord[firstCharPos]-'a');

	string word, prevWord = "";
	int switchFiles = 0;
	int switchComplement = 1;

	//Same work as createFinalTable().
	while(totalGuessed < l && totalMisses < 6){

		vector<int> alphaList;
		alphaList.resize(26, 0);
		vector<int> currentWord;

		int j;
		int len = dictionary[switchFiles].size();

		for(j = 0; j < len; j++){
			currentWord.clear();
			currentWord.resize(26, 0);
			word = dictionary[switchFiles][j];
			int l = word.size();
			
			if(!wordMatches(word, guessWord, guessesPos, misses)) continue;

			//write into file which will be read next
			dictionary[switchComplement].push_back(word);

			for(i = 0; i < l; i++){
				int ch = word[i]-'a';
				if(currentWord[ch] != 0) continue;
				alphaList[ch]++;
				currentWord[ch]++;
			}
		}

		int max = -1, maxVal = -1;
		for(i = 0; i < 26; i++){
			if(alphaList[i] > maxVal && hits[i] == 0){
				max = i;
				maxVal = alphaList[max];
			}
			alphaList[i] = 0;
		}

		dictionary[switchFiles].clear();
		switchFiles = (switchFiles+1)%2;
		switchComplement = (switchComplement+1)%2;

		if(maxVal <= 0){
			continue;
		}

		guessResult = fillUpWordAccToGuess((char)('a'+max), guessWord, answer, guessesPos);
		printSteps <<"Guess : "<<(char)('a'+max)<<endl;

		if(guessResult > 0){
			totalGuessed += guessResult;
			hits[max]++;
			printSteps <<answer<<endl;
			printSteps <<"Misses : "<<missPrint<<endl;
		}
		else{
			missPrint += (char)('a'+max);
			misses[max]++;
			totalMisses++;
			
			printSteps <<answer<<endl;
			printSteps <<"Misses : "<<missPrint<<endl;
		}

		printSteps<<"-------------------------------"<<endl;
	}

	//Returns 1 if the word was correctly guessed before having 6 misses else 0.
	if(totalGuessed == l){
		printSteps<<"Solved"<<endl<<"-------------------------------"<<endl<<"-------------------------------"<<endl;
		return 1;
	}
	else{
		printSteps<<"Failed"<<endl<<"-------------------------------"<<endl<<"-------------------------------"<<endl;
		return 0;
	}
}


int main(void){
	clock_t start_time = clock();
	
	maxWordLength = 0;

	makeVector();
	createFinalTable();

	printSteps.open("output.txt");

	int correctlyGuessed = 0;

	int i, len = all_words_with_duplicates.size();
	for(i = 0; i < len; i++){
		int solved = solver(all_words_with_duplicates[i]);
		if(solved) correctlyGuessed ++;
	}

	printSteps.close();

	clock_t end_time = clock();

	cout<<"Number of words tested : "<<len<<endl;
	cout<<"Number of words correctly guessed : "<<correctlyGuessed<<endl;
	cout<<"Accuracy in guessing : "<<((correctlyGuessed*100.0)/len)<<endl;
	cout<<"Time taken : "<<((end_time - start_time)/CLOCKS_PER_SEC)<<endl;
	
	return 0;
}