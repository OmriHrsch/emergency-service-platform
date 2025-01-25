package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;

public class StompMessageEncoderDecoder implements MessageEncoderDecoder<StompFrame> {
    private byte[] bytes = new byte[1024];
    private int len = 0;
    
    @Override
    public StompFrame decodeNextByte(byte nextByte){ //decoding a single byte, returns a frame if the byte completes a frame, otherwise returns null
        if (nextByte == 0) {
            String toFrame = new String(bytes,0,len);
            resetDecoder();
            return new StompFrame(toFrame);
        }
        pushByte(nextByte);
        return null; //not a line yet
    }

    private void pushByte(byte nextByte) { //pushes a byte to the bytes array
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    @Override
    public byte[] encode(StompFrame frame){ //encodes a frame to a byte array
        return frame.toString().getBytes(StandardCharsets.UTF_8);
    }    

    private void resetDecoder(){
        len = 0;
        bytes = new byte[1024];
    }
}



    



