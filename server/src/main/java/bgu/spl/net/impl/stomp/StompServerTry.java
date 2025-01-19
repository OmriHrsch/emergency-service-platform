package bgu.spl.net.impl.stomp;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class StompServerTry {
    public static void main(String[] args) {
        int port = 61613; // Default STOMP port
        
        System.out.println("Starting STOMP server on port " + port + "...");

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("STOMP server is running. Waiting for clients to connect...");

            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("Client connected from: " + clientSocket.getInetAddress());

                // Handle client communication in a separate thread
                new Thread(() -> handleClient(clientSocket)).start();
            }
        } catch (IOException e) {
            System.err.println("Error starting server: " + e.getMessage());
        }
    }

    private static void handleClient(Socket clientSocket) {
        try (
            InputStream input = clientSocket.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(input));
        ) {
            String frame;
            System.out.println("Listening for frames from client...");
            
            while ((frame = reader.readLine()) != null) {
                if (frame.trim().isEmpty()) {
                    System.out.println("Received an empty frame delimiter. Waiting for the next frame...");
                } else {
                    System.out.println("Received frame:\n" + frame);
                }
            }
        } catch (IOException e) {
            System.err.println("Error handling client: " + e.getMessage());
        } finally {
            try {
                clientSocket.close();
            } catch (IOException e) {
                System.err.println("Error closing client socket: " + e.getMessage());
            }
        }

        System.out.println("Client disconnected.");
    }
}