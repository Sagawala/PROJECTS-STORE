package com.SUZA.CONVERTOR.Model;

import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class CurrencyConversionRequest {
    private String baseCurrency;
    private String targetCurrency;
    private double amount;
    private String sourceCurrency;

    public CurrencyConversionRequest() {
        // Default constructor for JSON deserialization
    }

    public CurrencyConversionRequest(String baseCurrency, String targetCurrency, double amount) {
        this.baseCurrency = baseCurrency;
        this.targetCurrency = targetCurrency;
        this.amount = amount;
    }

    public String getSourceCurrency() {
        return sourceCurrency;
    }

    // Add getters and setters for all fields
}
