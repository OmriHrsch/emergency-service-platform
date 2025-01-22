package bgu.spl.net.impl.stomp;
import java.util.HashMap;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class StompProtocol implements StompMessagingProtocol<StompFrame>{

    private boolean shouldTerminate = false;
    private int connectionID;
    private ConnectionsImpl<StompFrame> connections;
    private HashMap<Integer, String> subscriptions; //Holds all the subscriptions of the client
    private Client clientInfo;
    int messageIDCounter;


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
        StompFrame responseFrame;
        String username = message.getHeaderValue("login");
        String password = message.getHeaderValue("passcode");
        //Checking if client already exists
        if ((clientInfo = connections.clients.get(username)) == null) { 
            // New user: create and add to clients
            clientInfo = new Client(username, password);
            connections.clients.put(username, clientInfo);
        } 
        //Case: "User already logged in"
        else if (clientInfo.isLoggedIn()) {
            responseFrame = new StompFrame("ERROR\nmessage:User already logged in.\n\n\0");
            connections.send(password, responseFrame);
            connections.disconnect(connectionID);
            shouldTerminate = true;
        }
        //Case: "Wrong password"
        else if (!clientInfo.getPassword().equals(password)) {
            responseFrame = new StompFrame("ERROR\nmessage:Wrong password.\n\n\0");
            connections.send(password, responseFrame);
            connections.disconnect(connectionID);
            shouldTerminate = true;
        }
        //Case: "Login Successful"
        else {
            clientInfo.setLoggedIn(true);
            responseFrame = new StompFrame("CONNECTED\nversion:1.2\n\n\0");
            connections.send(password, responseFrame);
        }
    }

    private void processSubscribe(StompFrame message){ //need to decide what to do if the client is already subscribed to the channel
        String channel = message.getHeaderValue("destination").substring(1);
        int id = Integer.parseInt(message.getHeaderValue("id"));
        int receipt = Integer.parseInt(message.getHeaderValue("receipt"));
        if (subscriptions.containsKey(id)){
            String frameString = "ERROR\nreceipt-id:" + receipt + "\nmessage:Client is already used this subscription ID.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            connections.disconnect(connectionID);
            shouldTerminate = true;
        }
        if (subscriptions.containsValue(channel)){
            String frameString = "ERROR\nreceipt-id:" + receipt + "\nmessage:Client is already subscribed to this channel.\n\n\0";
            StompFrame error = new StompFrame(frameString);
            connections.send(connectionID, error);
            connections.disconnect(connectionID);
            shouldTerminate = true;
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
            connections.disconnect(connectionID);
            shouldTerminate = true;
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
            connections.disconnect(connectionID);
            shouldTerminate = true;
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
        connections.disconnect(connectionID);
            shouldTerminate = true;
    }
}