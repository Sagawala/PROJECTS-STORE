package com.SUZA.CONVERTOR.Controller;

import com.SUZA.CONVERTOR.Model.CurrencyConversionRequest;
import com.SUZA.CONVERTOR.Model.CurrencyConversionResponse;
import com.SUZA.CONVERTOR.Service.CurrencyConverterService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@CrossOrigin(origins = "*", allowedHeaders = "*")
public class CurrencyConverterController {

    private final CurrencyConverterService converterService;

    @Autowired
    public CurrencyConverterController(CurrencyConverterService converterService) {
        this.converterService = converterService;
    }

    @PostMapping("/convert")
    public ResponseEntity<CurrencyConversionResponse> convertCurrency(@RequestBody CurrencyConversionRequest request) {
        CurrencyConversionResponse convertedAmount = converterService.convertCurrency(request);
        return ResponseEntity.ok(convertedAmount);
    }

}
