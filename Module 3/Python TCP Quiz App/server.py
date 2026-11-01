import socket
import json
import threading
from pathlib import Path

# Server configuration
HOST = "127.0.0.1"
PORT = 5000

# Load quiz questions from the JSON file
def load_questions():
    questions_file = Path(__file__).parent / "questions.json"

    with open(questions_file, "r", encoding="utf-8") as file:
        return json.load(file)


# Process requests received from the client
def handle_request(request, questions, session):

    action = request.get("action")

    # Request 1: Get the next quiz question
    if action == "get_question":

        if session["question_index"] >= len(questions):
            return {
                "success": True,
                "question": None,
                "message": "You have completed the quiz!"
            }

        question = questions[session["question_index"]]

        # Do not reveal the correct answer
        return {
            "success": True,
            "question": {
                "id": question["id"],
                "question": question["question"],
                "choices": question["choices"]
            }
        }

    # Request 2: Submit an answer
    elif action == "submit_answer":

        if session["question_index"] >= len(questions):
            return {
                "success": False,
                "error": "No more questions available."
            }

        question = questions[session["question_index"]]

        question_id = request.get("question_id")
        answer = request.get("answer")

        if question_id != question["id"]:
            return {
                "success": False,
                "error": "Incorrect question ID."
            }

        if not isinstance(answer, str) or not answer.strip():
            return {
                "success": False,
                "error": "Please provide an answer."
            }

        correct = (
            answer.strip().upper()
            == question["answer"].upper()
        )

        if correct:
            session["score"] += 1

        session["answered"] += 1
        session["question_index"] += 1

        return {
            "success": True,
            "correct": correct,
            "score": session["score"],
            "answered": session["answered"],
            "message": (
                "Correct!"
                if correct
                else "Incorrect! The answer was " + question["answer"]
            )
        }

    # Request 3: Check the current score
    elif action == "get_score":

        return {
            "success": True,
            "score": session["score"],
            "answered": session["answered"],
            "total_questions": len(questions)
        }

    # Handle unknown requests
    else:
        return {
            "success": False,
            "error": "Unknown request."
        }


# Handle an individual client connection
def handle_client(connection, address, questions):

    print(f"Connected to client: {address}")

    # Each client starts with a separate score
    session = {
        "question_index": 0,
        "score": 0,
        "answered": 0
    }

    try:
        with connection:

            while True:

                data = connection.recv(4096)

                if not data:
                    break

                # Convert received JSON into a Python dictionary
                try:
                    request = json.loads(data.decode("utf-8"))

                    response = handle_request(
                        request,
                        questions,
                        session
                    )

                except (json.JSONDecodeError, UnicodeDecodeError):
                    response = {
                        "success": False,
                        "error": "Invalid JSON request."
                    }

                # Convert response into JSON and send it back
                message = json.dumps(response) + "\n"

                connection.sendall(message.encode("utf-8"))

    except OSError as error:
        print(f"Connection error: {error}")

    finally:
        print(f"Client disconnected: {address}")


# Start the server
def main():

    questions = load_questions()

    server = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM
    )

    server.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_REUSEADDR,
        1
    )

    server.bind((HOST, PORT))

    server.listen()

    print(f"Quiz server listening on {HOST}:{PORT}")
    print("Waiting for clients...")

    try:
        while True:

            connection, address = server.accept()

            # Handle each client in a separate thread
            client_thread = threading.Thread(
                target=handle_client,
                args=(connection, address, questions)
            )

            client_thread.start()

    except KeyboardInterrupt:
        print("\nShutting down server.")

    finally:
        server.close()


if __name__ == "__main__":
    main()
