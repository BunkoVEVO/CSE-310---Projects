#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <cctype>

using namespace std;

// Represents the current state of a Hangman game.
class HangmanGame
{
private:
    string secretWord;
    string guessedWord;
    vector<char> guessedLetters;
    int incorrectGuesses;
    const int maxIncorrectGuesses = 6;

public:
    // Creates a new game using the selected word.
    HangmanGame(string word)
    {
        secretWord = word;
        guessedWord = string(word.length(), '_');
        incorrectGuesses = 0;
    }

    // Checks whether a letter has already been guessed.
    bool alreadyGuessed(char letter)
    {
        for (char guessed : guessedLetters)
        {
            if (guessed == letter)
            {
                return true;
            }
        }

        return false;
    }

    // Processes a player's letter guess.
    bool guessLetter(char letter)
    {
        letter = tolower(letter);

        if (alreadyGuessed(letter))
        {
            cout << "You already guessed that letter.\n";
            return false;
        }

        guessedLetters.push_back(letter);

        bool found = false;

        for (size_t i = 0; i < secretWord.length(); i++)
        {
            if (tolower(secretWord[i]) == letter)
            {
                guessedWord[i] = secretWord[i];
                found = true;
            }
        }

        if (!found)
        {
            incorrectGuesses++;
            cout << "Incorrect guess!\n";
        }
        else
        {
            cout << "Good guess!\n";
        }

        return found;
    }

    // Determines whether the player has won.
    bool hasWon()
    {
        return guessedWord == secretWord;
    }

    // Determines whether the player has lost.
    bool hasLost()
    {
        return incorrectGuesses >= maxIncorrectGuesses;
    }

    // Displays the current game state.
    void displayGame()
    {
        cout << "\n-----------------------------\n";
        cout << "Word: ";

        for (char letter : guessedWord)
        {
            cout << letter << " ";
        }

        cout << "\nIncorrect guesses: "
             << incorrectGuesses << "/" << maxIncorrectGuesses << "\n";

        cout << "Guessed letters: ";

        for (char letter : guessedLetters)
        {
            cout << letter << " ";
        }

        cout << "\n";

        displayHangman();
        cout << "-----------------------------\n";
    }

    // Displays an ASCII representation of the Hangman.
    void displayHangman()
    {
        cout << "\n";

        switch (incorrectGuesses)
        {
        case 0:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;

        case 1:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;

        case 2:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << "  |   |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;

        case 3:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|   |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;

        case 4:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;

        case 5:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << " /    |\n";
            cout << "=========\n";
            break;

        case 6:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << " / \\  |\n";
            cout << "=========\n";
            break;
        }
    }

    // Displays the secret word after the game ends.
    void displayAnswer()
    {
        cout << "The word was: " << secretWord << "\n";
    }
};

// Loads possible words from a text file.
vector<string> loadWords(string filename)
{
    vector<string> words;
    ifstream inputFile(filename);

    if (!inputFile)
    {
        cout << "Error: Could not open " << filename << ".\n";
        return words;
    }

    string word;

    while (inputFile >> word)
    {
        words.push_back(word);
    }

    inputFile.close();

    return words;
}

// Selects a random word from the list.
string chooseRandomWord(const vector<string>& words)
{
    random_device randomDevice;
    mt19937 generator(randomDevice());

    uniform_int_distribution<int> distribution(
        0,
        static_cast<int>(words.size()) - 1
    );

    return words[distribution(generator)];
}

// Gets a valid letter from the player.
char getLetterGuess()
{
    char letter;

    cout << "Enter a letter: ";
    cin >> letter;

    return static_cast<char>(tolower(letter));
}

// Plays one complete game.
void playGame(const vector<string>& words)
{
    string selectedWord = chooseRandomWord(words);

    HangmanGame game(selectedWord);

    cout << "\n=============================\n";
    cout << "       HANGMAN GAME\n";
    cout << "=============================\n";

    while (!game.hasWon() && !game.hasLost())
    {
        game.displayGame();

        char guess = getLetterGuess();

        game.guessLetter(guess);
    }

    game.displayGame();

    if (game.hasWon())
    {
        cout << "\nCongratulations! You guessed the word!\n";
    }
    else
    {
        cout << "\nGame over! Better luck next time.\n";
        game.displayAnswer();
    }
}

// Displays the main menu.
void displayMenu()
{
    cout << "\n=============================\n";
    cout << "       HANGMAN MENU\n";
    cout << "=============================\n";
    cout << "1. Play Game\n";
    cout << "2. Exit\n";
    cout << "Choose an option: ";
}

// Main program.
int main()
{
    vector<string> words = loadWords("words.txt");

    if (words.empty())
    {
        cout << "No words were loaded. Please check words.txt.\n";
        return 1;
    }

    bool running = true;

    while (running)
    {
        displayMenu();

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            playGame(words);
        }
        else if (choice == 2)
        {
            cout << "Thanks for playing!\n";
            running = false;
        }
        else
        {
            cout << "Invalid option. Please try again.\n";
        }
    }

    return 0;
}