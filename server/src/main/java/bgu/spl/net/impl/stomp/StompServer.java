package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if(args.length != 2) //Is this check needed?
        {
            throw new IllegalArgumentException("Args should be: port, server-type");
        }
        Server<StompFrame> server = null;
        if(args[1].equals("tpc")) {
            server = Server.threadPerClient(Integer.parseInt(args[0]),
                    () -> new StompProtocol(),
                    ()-> new StompMessageEncoderDecoder(),
                    new ConnectionsImpl<StompFrame>());
        }
        else if(args[1].equals("reactor")) {
            server = Server.reactor(4, // TODO: Check how many threads should be used
                    Integer.parseInt(args[0]),
                    () -> new StompProtocol(),
                    () -> new StompMessageEncoderDecoder(),
                    new ConnectionsImpl<StompFrame>());
        }
        server.serve();

    }
    
}
