package bgu.spl.net.impl.stomp;

public class Client {
    private String username;
    private String password;
    private boolean isLoggedIn;

    public Client(String username, String password) {
        this.username = username;
        this.password = password;
        this.isLoggedIn = false;
    }

    //Getters
    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }
    
    public boolean isLoggedIn() {
        return isLoggedIn;
    }

    //Setters
    public void setUsername(String username) {
        this.username = username;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public void setLoggedIn(boolean loggedIn) {
        isLoggedIn = loggedIn;
    }
    
    public boolean equals(Client other) {
        return (username.equals(other.getUsername()) && password.equals(other.getPassword()));
    }
}