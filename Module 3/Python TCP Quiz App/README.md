
# Python TCP Quiz Application

## Project Overview

This project is a client-server quiz application written in Python.

The client and server communicate using TCP sockets. They exchange information using JSON messages.

The server loads quiz questions from a local JSON file, checks submitted answers, and tracks the user's score.

## Requirements

- Python 3
- No external libraries required

## Project Files

- `server.py`: Handles client connections and processes requests.
- `client.py`: Connects to the server and provides the quiz interface.
- `questions.json`: Contains quiz questions and correct answers.

## How to Run

1. Open a terminal in the project folder.
2. Start the server:

   `python server.py`

3. Open a second terminal in the same folder.
4. Start the client:

   `python client.py`

5. Use the client menu to start the quiz, check your score, or exit.

## Networking Concepts

### TCP

TCP (Transmission Control Protocol) allows the client and server to communicate reliably over a network.

### IP Address

The application uses `127.0.0.1`, also known as localhost. This allows both programs to communicate on the same computer.

### Ports

The server listens on port `5000`. The client uses this port to establish its connection.

### JSON

JSON is used to exchange information between the client and server.

The client sends JSON requests, and the server processes them and returns JSON responses.

## Client Requests

The application supports three request types:

1. `get_question`: Requests the next quiz question.
2. `submit_answer`: Submits an answer for the current question.
3. `get_score`: Requests the current quiz score.

## Testing

The application can be tested by running the server and client in separate terminal windows.

Testing should include:

- Establishing a successful connection.
- Retrieving quiz questions.
- Submitting correct answers.
- Submitting incorrect answers.
- Checking the current score.
- Attempting to connect when the server is not running.
- Entering invalid menu selections.

## Limitations

Scores are stored in memory and are not saved after a client disconnects.

The application currently uses localhost, so both programs are intended to run on the same computer.
