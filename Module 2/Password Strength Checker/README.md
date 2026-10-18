# Password Strength Checker

## Project Description

The Password Strength Checker is a simple web application built with JavaScript, HTML, and CSS. It allows users to enter a sample password and receive an estimate of its strength based on several password requirements.

This project was created for CSE 310 – Applied Programming to practice JavaScript programming and web development.

## Features

- Checks whether a password is at least 12 characters long.
- Checks for uppercase and lowercase letters.
- Checks for numbers and special characters.
- Calculates a password score out of 100.
- Displays a strength rating: Weak, Fair, Good, or Strong.
- Shows a checklist of requirements the password meets.
- Provides suggestions for improving password strength.
- Includes a button to show or hide the password.
- Uses a responsive layout for desktop and mobile devices.
- Checks passwords locally in the browser without sending them to a server.

## Technologies Used

- **HTML5:** Creates the webpage structure.
- **CSS3:** Styles the interface and makes the layout responsive.
- **JavaScript:** Checks password requirements, calculates scores, and updates the webpage.

## Project Files

```text
password-strength-checker/
├── index.html
├── style.css
├── script.js
└── README.md
```

## How to Run

1. Download or clone the project repository.
2. Open the project folder.
3. Open `index.html` in a web browser.
4. Enter a sample password in the input field.
5. Click **Check Password** to see the results.
6. Review the strength rating, requirements checklist, and improvement suggestions.

No additional software or package installation is required.

## How It Works

The JavaScript program uses functions, arrays, loops, conditional statements, and regular expressions to evaluate the password.

It checks five requirements:

1. At least 12 characters.
2. At least one uppercase letter.
3. At least one lowercase letter.
4. At least one number.
5. At least one special character.

The program calculates a score based on password length and the requirements met. It then displays a rating and suggestions based on the results.

## Testing

The following cases can be used to test the application:

- Submit an empty password.
- Enter a short password.
- Enter a password containing only lowercase letters.
- Enter a password containing letters, numbers, and special characters.
- Enter a longer password that meets all five requirements.
- Test the show/hide password button.
- Check that the page works on different screen sizes.

## Limitations

The score is an educational estimate and does not guarantee that a password is secure. The application does not check whether a password has appeared in a data breach or measure its resistance to real-world guessing attacks.

Passwords are evaluated locally in the browser. The application does not intentionally store or transmit them to a server.

## Author

Thomas Hail

CSE 310 – Applied Programming

## License

This project was created for educational purposes.
