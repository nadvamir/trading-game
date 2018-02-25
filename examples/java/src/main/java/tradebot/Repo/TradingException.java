public class TradingException extends RuntimeException {
    public TradingException(String message){
        super(message);
     }
    public TradingException(Exception e){
        super(e);
     }
}