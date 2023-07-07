package com.SUZA.CONVERTOR.Service;




import com.SUZA.CONVERTOR.Model.CurrencyConversionRequest;
import com.SUZA.CONVERTOR.Model.CurrencyConversionResponse;
import com.SUZA.CONVERTOR.Util.ExchangeRateAPIUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class CurrencyConverterService {

    private final ExchangeRateAPIUtil exchangeRateAPIUtil;

    @Autowired
    public CurrencyConverterService(ExchangeRateAPIUtil exchangeRateAPIUtil) {
        this.exchangeRateAPIUtil = exchangeRateAPIUtil;
    }

    public CurrencyConversionResponse convertCurrency(CurrencyConversionRequest request) {
        double exchangeRate = exchangeRateAPIUtil.getExchangeRate(request.getSourceCurrency(), request.getTargetCurrency());
        double convertedAmount = request.getAmount() * exchangeRate;

        CurrencyConversionResponse response = new CurrencyConversionResponse();
        response.setConvertedAmount(convertedAmount);

        return response;
    }
}
