
import socket
import json

# Server configuration
HOST = "127.0.0.1"
PORT = 5000


# Send a request and receive the server's response
def send_request(connection, request):

    # Convert Python dictionary into JSON
    message = json.dumps(request) + "\n"

    connection.sendall(message.encode("utf-8"))

    # Receive the response until the newline arrives
    response_data = b""

    while b"\n" not in response_data:

        data = connection.recv(4096)

        if not data:
            raise ConnectionError("Server disconnected.")

        response_data += data

    response_line = response_data.split(b"\n", 1)[0]

    return json.loads(response_line.decode("utf-8"))


# Display the current score
def check_score(connection):

    response = send_request(
        connection,
        {"action": "get_score"}
    )

    if response["success"]:

        print("\nYour current score:")
        print(
            f"{response['score']} correct out of "
            f"{response['answered']} answered."
        )

    else:
        print("Error:", response["error"])


# Run the quiz
def start_quiz(connection):

    print("\nWelcome to the Networking Quiz!")

    while True:

        # Request a question from the server
        response = send_request(
            connection,
            {"action": "get_question"}
        )

        if not response["success"]:
            print("Error:", response["error"])
            break

        question = response["question"]

        # Check whether the quiz is finished
        if question is None:
            print("\nYou completed the quiz!")
            break

        print("\n" + question["question"])

        # Display answer choices
        for choice in question["choices"]:
            print(choice)

        answer = input("\nYour answer: ").strip()

        # Submit the answer to the server
        result = send_request(
            connection,
            {
                "action": "submit_answer",
                "question_id": question["id"],
                "answer": answer
            }
        )

        print("\n" + result["message"])

        if not result["success"]:
            break

        print(
            f"Score: {result['score']}/{result['answered']}"
        )

    check_score(connection)


# Main client program
def main():

    print("Connecting to quiz server...")

    try:

        # Connect to the server
        with socket.create_connection(
            (HOST, PORT),
            timeout=10
        ) as connection:

            print("Successfully connected!")

            while True:

                print("\n===== QUIZ MENU =====")
                print("1. Start Quiz")
                print("2. Check Score")
                print("3. Exit")

                choice = input("Select an option: ")

                if choice == "1":

                    start_quiz(connection)

                elif choice == "2":

                    check_score(connection)

                elif choice == "3":

                    print("Goodbye!")
                    break

                else:

                    print("Invalid selection. Try again.")

    except ConnectionRefusedError:

        print("Could not connect to the server.")
        print("Make sure server.py is running.")

    except (ConnectionError, OSError) as error:

        print("Network error:", error)


if __name__ == "__main__":
    main()
