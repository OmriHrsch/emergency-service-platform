package bgu.spl.net.impl.stomp;

import java.util.HashMap;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class StompProtocol implements StompMessagingProtocol<StompFrame>{

    private boolean shouldTerminate = false;
    private int connectionID;
    private ConnectionsImpl<StompFrame> connections;
    private ConnectionHandler<StompFrame> handler;
    private HashMap<Integer, String> subscriptions;
    private int messageIDCounter;


	@Override
    public void start(int connectionId, Connections<StompFrame> connections) { //Must finish before using process so we could rub login logic as intended
        this.connectionID = connectionId;
        this.connections = (ConnectionsImpl<StompFrame>) connections;
        this.subscriptions = new HashMap<>();
        this.messageIDCounter = 0;
    }

    @Override
    public void process(StompFrame message) {
        String command = message.getCommand();
        if (command.equals("CONNECT"))
            processConnect(message);
        else if (command.equals("SUBSCRIBE"))
            processSubscribe(message);
        else if (command.equals("UNSUBSCRIBE"))
            processUnsubscribe(message);
        else if (command.equals("SEND"))
            processSend(message);
        else if (command.equals("DISCONNECT"))
            processDisconnect(message);
    }
    
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    //Helper proccesing methods
     private void processConnect(StompFrame message){
        String username = message.getHeaderValue("login");
        String password = message.getHeaderValue("passcode");
        //Case: "The client is already logged in, log out before trying again"
        if (connections.connections.containsKey(connectionID)){
            StompFrame error = new StompFrame("ERROR\nmessage:The client is already logged in, log out before trying again\n\n\0");
            connections.send(connectionID, error);
            //Sending back an error frame isnt a must, but if we do so - the connection must be closed. do we want to send a frmae here? 
            //If not, how do we transfer the error desctiption to the client?
        }
        //Case: "Login Successful"
        else if
        //Case: "User already logged in"

        //Case: "Wrong password"

    }

    private void processSubscribe(StompFrame message){ //need to decide what to do if the client is already subscribed to the channel
        String channel = message.getHeaderValue("destination").substring(1);
        int id = Integer.parseInt(message.getHeaderValue("id"));
        int receipt = Integer.parseInt(message.getHeaderValue("receipt"));
        if (subscriptions.containsKey(id)){
            String frameString = "ERROR\nreceipt-id:" + receipt + "\nmessage:Client is already used this subscription ID.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            disconnect();
        }
        if (subscriptions.containsValue(channel)){
            String frameString = "ERROR\nreceipt-id:" + receipt + "\nmessage:Client is already subscribed to this channel.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            disconnect();
        }
        else{
            subscriptions.put(id, channel);
            connections.subscribe(connectionID, channel);
            String frameString = "RECEIPT\nreceipt-id:" + receipt + "\n\n\0";
            StompFrame receiptFrame = new StompFrame(frameString);
            connections.send(connectionID, receiptFrame);
        }
    }

    private void processUnsubscribe(StompFrame message){
        if (!subscriptions.containsKey(Integer.parseInt(message.getHeaderValue("id")))){
            String frameString = "ERROR\nreceipt-id:" + message.getHeaderValue("receipt") + "\nmessage:Client is not subscribed to this channel.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            disconnect();
        }
        else {
            int id = Integer.parseInt(message.getHeaderValue("id"));
            String channel = subscriptions.get(id);
            subscriptions.remove(id);
            connections.unsubscribe(connectionID, channel);
            String frameString = "RECEIPT\nreceipt-id:" + message.getHeaderValue("receipt") + "\n\n\0";
            StompFrame receipt = new StompFrame(frameString);
            connections.send(connectionID, receipt);
        }
    }

    private void processSend(StompFrame message){
        String channel = message.getHeaderValue("destination").substring(1);
        String body = message.getBody();
        if (!subscriptions.containsValue(channel)){
            String frameString = "ERROR\nmessage:Client is not subscribed to this channel.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            disconnect();
        }
        else{
            String frameString = "MESSAGE\ndestination:" + channel + "message-id:" + messageIDCounter + "\n\n" + body + "\0";
            messageIDCounter++;
            StompFrame frame = new StompFrame(frameString);
            connections.send(channel, frame);
        }
    }

    private void processDisconnect(StompFrame message){
        String receipt = message.getHeaderValue("receipt");
        String frameString = "RECEIPT\nreceipt-id:" + receipt + "\n\n\0";
        StompFrame receiptFrame = new StompFrame(frameString);
        connections.send(connectionID, receiptFrame); 
        disconnect();
    }

    private void disconnect(){
        connections.disconnect(connectionID);
        shouldTerminate = true;
    }
}