package org.boilermake.boilersnap;

/**
 * Created by rohan on 17/10/15.
 */
public class Payload {
    private String toUsername;
    private String fromUsername;
    private String image1;
    private String image2;

    public Payload() {
        /*
            nothing
         */
    }
    public Payload(String toUsername, String fromUsername, String image1, String image2) {
        this.toUsername = toUsername;
        this.fromUsername = fromUsername;
        this.image1 = image1;
        this.image2 = image2;
    }
}
