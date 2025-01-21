package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;

public class StompMessageEncoderDecoder implements MessageEncoderDecoder<StompFrame> {
    private byte[] bytes = new byte[1024];
    private int len = 0;
    
    @Override
    public StompFrame decodeNextByte(byte nextByte){
        if (nextByte == '\0') {
            String toFrame = new String(bytes,0,len);
            resetDecoder();
            return new StompFrame(toFrame);
        }
        pushByte(nextByte);
        return null; //not a line yet
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    @Override
    public byte[] encode(StompFrame frame){
        return frame.toString().getBytes(StandardCharsets.UTF_8);
    }    

    private void resetDecoder(){
        len = 0;
        bytes = new byte[1024];
    }
}



    



