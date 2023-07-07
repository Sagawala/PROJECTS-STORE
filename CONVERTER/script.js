function convertCurrency() {
    var baseCurrency = document.getElementById("baseCurrency").value;
    var targetCurrency = document.getElementById("targetCurrency").value;
    var amount = document.getElementById("amount").value;

    var request = {
        baseCurrency: baseCurrency,
        targetCurrency: targetCurrency,
        amount: parseFloat(amount)
    };

    fetch("http://localhost:8080/convert", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(request)
    })
    .then(response => {
        if (!response.ok) {
            throw new Error("Failed to convert currency");
        }
        return response.json();
    })
    .then(data => {
        document.getElementById("convertedAmount").innerHTML = "Converted Amount: " + data;
    })
    .catch(error => {
        console.error(error);
        document.getElementById("convertedAmount").innerHTML = "Failed to convert currency";
    });
}
