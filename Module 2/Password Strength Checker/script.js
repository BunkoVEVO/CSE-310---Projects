
"use strict";

// Get the HTML elements we need.
const form = document.querySelector("#password-form");
const passwordInput = document.querySelector("#password");
const toggleButton = document.querySelector("#toggle-password");
const results = document.querySelector("#results");
const strengthLabel = document.querySelector("#strength-label");
const strengthBar = document.querySelector("#strength-bar");
const strengthTrack = document.querySelector("#strength-track");
const scoreText = document.querySelector("#score-text");
const feedback = document.querySelector("#feedback");

// Define the password requirements.
const requirements = [
    {
        name: "length",
        description: "Use at least 12 characters.",
        check: password => password.length >= 12
    },
    {
        name: "uppercase",
        description: "Add an uppercase letter (A–Z).",
        check: password => /[A-Z]/.test(password)
    },
    {
        name: "lowercase",
        description: "Add a lowercase letter (a–z).",
        check: password => /[a-z]/.test(password)
    },
    {
        name: "number",
        description: "Add a number (0–9).",
        check: password => /[0-9]/.test(password)
    },
    {
        name: "special",
        description: "Add a special character, such as !, @, or #.",
        check: password => /[^A-Za-z0-9]/.test(password)
    }
];

// Check which requirements a password meets.
function checkRequirements(password) {
    return requirements.map(requirement => ({
        ...requirement,
        met: requirement.check(password)
    }));
}

// Calculate a score out of 100.
function calculateScore(password, checks) {
    if (password.length === 0) {
        return 0;
    }

    let score = 0;

    // Reward longer passwords.
    if (password.length >= 8) {
        score += 10;
    }

    if (password.length >= 12) {
        score += 20;
    }

    // Award points for each requirement that is met.
    for (const requirement of checks) {
        if (requirement.met &&
            requirement.name !== "length") {
            score += 15;
        }
    }

    // Give a bonus for extra length.
    if (password.length >= 16) {
        score += 10;
    }

    // Keep the score between 0 and 100.
    return Math.min(score, 100);
}

// Convert the numerical score into a rating.
function getStrength(score) {
    if (score < 30) {
        return {
            label: "Weak",
            className: "weak"
        };
    }

    if (score < 60) {
        return {
            label: "Fair",
            className: "fair"
        };
    }

    if (score < 85) {
        return {
            label: "Good",
            className: "good"
        };
    }

    return {
        label: "Strong",
        className: "strong"
    };
}

// Update the requirements checklist on the page.
function displayRequirements(checks) {
    for (const requirement of checks) {
        const item = document.querySelector(
            `[data-rule="${requirement.name}"]`
        );

        const status = item.querySelector(".status");

        item.classList.toggle("met", requirement.met);

        status.textContent = requirement.met ? "✓" : "○";
    }
}

// Create suggestions based on missing requirements.
function createFeedback(password, checks) {
    if (password.length === 0) {
        return "Enter a password to see suggestions.";
    }

    const suggestions = [];

    // Check for missing requirements.
    for (const requirement of checks) {
        if (!requirement.met) {
            suggestions.push(requirement.description);
        }
    }

    if (password.length < 16) {
        suggestions.push(
            "Consider using a longer password or passphrase."
        );
    }

    if (suggestions.length === 0) {
        return "Your password meets all the listed requirements. " +
            "Remember that length and unpredictability matter too.";
    }

    return suggestions.join(" ");
}

// Display the score, rating, checklist, and feedback.
function displayResults(password) {
    const checks = checkRequirements(password);
    const score = calculateScore(password, checks);
    const strength = getStrength(score);

    // Show the results section.
    results.hidden = false;

    // Update the strength rating.
    strengthLabel.textContent = strength.label;

    // Update the progress bar.
    strengthBar.style.width = `${score}%`;
    strengthTrack.setAttribute("aria-valuenow", score);
    strengthTrack.setAttribute(
        "aria-valuetext",
        `${score} out of 100, ${strength.label}`
    );

    // Update the progress bar color.
    strengthBar.className = `strength-bar ${strength.className}`;

    // Display the numerical score.
    scoreText.textContent = `Score: ${score} out of 100`;

    // Update the checklist and suggestions.
    displayRequirements(checks);
    feedback.textContent = createFeedback(password, checks);
}

// Check the password when the form is submitted.
form.addEventListener("submit", function (event) {
    event.preventDefault();

    const password = passwordInput.value;

    if (password.length === 0) {
        results.hidden = false;
        strengthLabel.textContent = "Not checked";
        strengthBar.style.width = "0%";
        strengthBar.className = "strength-bar";
        strengthTrack.setAttribute("aria-valuenow", 0);
        strengthTrack.setAttribute(
            "aria-valuetext",
            "No password entered"
        );
        scoreText.textContent = "Please enter a password first.";
        displayRequirements(checkRequirements(""));
        feedback.textContent =
            "Enter a password to see suggestions.";

        passwordInput.focus();
        return;
    }

    displayResults(password);
});

// Show or hide the password.
toggleButton.addEventListener("click", function () {
    const isHidden = passwordInput.type === "password";

    passwordInput.type = isHidden ? "text" : "password";
    toggleButton.textContent = isHidden ? "Hide" : "Show";
    toggleButton.setAttribute("aria-pressed", String(isHidden));
    toggleButton.setAttribute(
        "aria-label",
        isHidden ? "Hide password" : "Show password"
    );
});
