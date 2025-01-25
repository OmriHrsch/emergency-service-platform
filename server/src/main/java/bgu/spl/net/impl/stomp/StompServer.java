package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if(args.length != 2) 
        {
            throw new IllegalArgumentException("Args should be: port, server-type");
        }
        Server<StompFrame> server = null;
        if(args[1].equals("tpc")) { //Initialize the server as a thread per client server
            server = Server.threadPerClient(Integer.parseInt(args[0]),
                    () -> new StompProtocol(),
                    ()-> new StompMessageEncoderDecoder(),
                    new ConnectionsImpl<StompFrame>());
        }
        else if(args[1].equals("reactor")) { //Initialize the server as a reactor server
            int numberOfCores = Runtime.getRuntime().availableProcessors();
            server = Server.reactor(numberOfCores, 
                    Integer.parseInt(args[0]),
                    () -> new StompProtocol(),
                    () -> new StompMessageEncoderDecoder(),
                    new ConnectionsImpl<StompFrame>());
        }
        server.serve();

    }
    
}
