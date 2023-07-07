package com.SUZA.CONVERTOR.Util;



import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

@Component
public class ExchangeRateAPIUtil {

    @Value("${exchange.rate}")
    private double exchangeRate;

    public double getExchangeRate(String sourceCurrency, String targetCurrency) {
        // You can add additional logic here if needed, such as handling different currency pairs
        return exchangeRate;
    }
}
