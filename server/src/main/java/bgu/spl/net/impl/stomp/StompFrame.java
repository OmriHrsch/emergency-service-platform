package bgu.spl.net.impl.stomp;
import java.util.*;

public class StompFrame {
    private String command;
    private HashMap<String, String> header;
    private String body;

    // Constructor to parse a frame string
    public StompFrame(String frameString) {
        this.command = "";
        this.header = new HashMap<>();
        this.body = "";

        String[] lines = frameString.split("\n");
        boolean inBody = false;
        StringBuilder bodyContent = new StringBuilder();

        for (String line : lines) {
            if (line.isEmpty()) {
                inBody = true; // Empty line indicates the start of the body
                continue;
            }

            if (!inBody) {
                // Parse the command (first non-empty line is the command)
                if (this.command.isEmpty()) {
                    this.command = line;
                } else {
                    // Parse header lines
                    List<String> headerParts = strSplit(line, ':');
                    if (headerParts.size() == 2) {
                        String headerName = headerParts.get(0);
                        String headerValue = headerParts.get(1);
                        this.header.put(headerName, headerValue);
                    }
                }
            } else {
                // Accumulate body lines
                bodyContent.append(line).append("\n");
            }
        }

        // Remove the trailing '\n' from bodyContent if present
        if (bodyContent.length() > 0 && bodyContent.charAt(bodyContent.length() - 1) == '\n') {
            bodyContent.setLength(bodyContent.length() - 1);
        }

        this.body = bodyContent.toString();
    }

    // Utility method to split a string by a delimiter
    private List<String> strSplit(String word, char delimiter) {
        List<String> toReturn = new ArrayList<>();
        int index = word.indexOf(delimiter);
        if (index != -1) {
            toReturn.add(word.substring(0, index));
            toReturn.add(word.substring(index + 1));
        }
        return toReturn;
    }

    // Converts the object to a string representation of the frame
    public String toString() {
        StringBuilder frameString = new StringBuilder();
        frameString.append(command).append("\n");
        for (Map.Entry<String, String> pair : header.entrySet()) {
            frameString.append(pair.getKey()).append(":").append(pair.getValue()).append("\n");
        }
        frameString.append("\n");
        frameString.append(body);
        return frameString.toString();
    }

    // Getters
    public String getCommand() {
        return command;
    }

    public HashMap<String, String> getHeader() {
        return header;
    }

    public String getHeaderValue(String key) {
        return header.get(key);
    }

    public String getBody() {
        return body;
    }
}
