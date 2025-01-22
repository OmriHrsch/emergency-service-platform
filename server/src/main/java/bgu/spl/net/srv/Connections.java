package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    public void addConnection(int connectionId, ConnectionHandler<T> handler); //Added this method to the interface to allow using it through the connection handlers
}
