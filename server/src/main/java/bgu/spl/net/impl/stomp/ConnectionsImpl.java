package bgu.spl.net.impl.stomp;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;


public class ConnectionsImpl<T> implements Connections<T> {

    final ConcurrentHashMap <Integer, ConnectionHandler<T>> connections; //Saves all ACTIVE connections
    final ConcurrentHashMap <String, List<Integer>> channelSubscriptions; //Saves all the channels and the connections that are subscribed to them
    final ConcurrentHashMap <String, Client> clients; //A client database 

    public ConnectionsImpl() { //Constructor
        connections = new ConcurrentHashMap<>();
        channelSubscriptions = new ConcurrentHashMap<>();
        clients = new ConcurrentHashMap<>();
    }


    @Override
    public boolean send(int connectionId, T msg){ //Sending a message to a specific connection
        ConnectionHandler<T> handler = connections.get(connectionId);
        if (handler == null){   
            return false;
        }
        handler.send(msg);
        return true;
    }

    @Override
    public void send(String channel, T msg){ //Sending a message to all the connections that are subscribed to a specific channel
        List<Integer> subscribedToChannel = channelSubscriptions.get(channel);
        if (subscribedToChannel!=null){
            for (int connection : subscribedToChannel){
                send(connection, msg);
            }
        }
    }
        
    
    @Override
    public void disconnect(int connectionId){
        ConnectionHandler<T> toDelete = connections.remove(connectionId); //Removing the connection from the connections map
        if (toDelete != null){
            channelSubscriptions.values().forEach(subscribers -> subscribers.remove((Integer) connectionId)); //Removing the connection from all the channels it is subscribed to
        }
        else {
            System.out.println("Connection not found\n");
        }
    }


    public void addConnection(int connectionId, ConnectionHandler<T> handler) { //Adding a connection to the connections map
        connections.put(connectionId, handler);
    }

    public void subscribe(int connectionId, String channel) { //Adding a connection to a channel
        channelSubscriptions.computeIfAbsent(channel, k -> new CopyOnWriteArrayList<>()).add(connectionId);
    }

    public void unsubscribe(int connectionId, String channel) { //Removing a connection from a channel
        List<Integer> subscribers = channelSubscriptions.get(channel);
        if (subscribers != null){
            subscribers.remove((Integer) connectionId);
        }
    }
}
