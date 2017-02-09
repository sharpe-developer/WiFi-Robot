/******************************************************************************
 * NAME: UdpSocket
 * 
 * DESCRIPTION:
 *   Class for handling a single UDP socket. Creates the socket using the 
 *   specified port (or ephemeral if no port specified). Implements basic 
 *   send and receive functionality for the socket. Creates a thread 
 *   to process incoming received messages and places the received messages
 *   in a queue for the user.
 *****************************************************************************/
package com.sharpedev.robotremote;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.LinkedList;
import java.util.Queue;
import android.util.Log;

public class UdpSocket implements Runnable {
    
    private final String TAG = this.getClass().getSimpleName();
    
    boolean        isRunning    = false;
    boolean        isConnected  = false;    
    byte[]         rxBuffer     = new byte[1500];
    DatagramSocket socket       = null;
    DatagramPacket packet       = null;
    Thread         rxThread     = null;
    int            localPort    = 0;
    int            remotePort   = 0;
    InetAddress    remoteIp     = null;
    
    //Queue for received messages
    Queue<DatagramPacket> packetQueue = new LinkedList<DatagramPacket>();;      
    
    /**
     * Default Class constructor
     */
    public UdpSocket() {
        
        //Let the OS assign an ephemeral port for us
        localPort = 0;      
    }
    
    /**
     * Constructs a UDP socket using the specified port
     * 
     * @param port - the local UDP port for the UDP socket
     */
    public UdpSocket(int port) {
        
        //Use the specified port for the local port
        localPort = port;       
    }   
    
    /**
     * Set the remote end point IP address and port. No connection 
     * actually takes place. This method allows the socket to set
     * the end point parameters once instead of passing them repeatedly 
     * to the sendto method.
     * 
     * @param ip - remote IP address
     * @param port - remote port
     */
    public void connect(InetAddress ip, int port) {
        
        //No actual connection. 
        //Just record the remote end point information
        remoteIp    = ip;
        remotePort  = port; 
        isConnected = true;
    }
    
    /**
     * Open the datagram socket and start the receive thread
     */
    public void start() {
        
        //Log.d(TAG, "Starting UDP");
                
        try {
            
            //Create the socket
            socket = new DatagramSocket(localPort);
            
            //Start the receive thread
            rxThread = new Thread(this);    
            rxThread.start();
            
        } 
        catch (SocketException e) {
            
            Log.e(TAG, "UDP Client start exception");
            e.printStackTrace();
        }               
    }
    
    /**
     * Stop the receive thread and close the socket if not already closed
     */
    public void stop() {
        
        //Log.d(TAG, "Stopping UDP");
        
        //Stop the receive thread
        isRunning = false;  
        rxThread.interrupt();
        
        //Close the socket
        if (socket != null) {
                        
            socket.close();
        }
    }
    
    /** 
     * The UDP socket receive thread. This thread waits for received packets 
     * on the socket and inserts the received packet in a queue for the user
     * to retrieve 
     * 
     * @see java.lang.Runnable#run()
     */
    @Override
    public void run() {
        
        //Run the receive thread in the background
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);            

        try  {                                            
                        
            Log.d(TAG, "Enterring UDP receive loop");
            
            isRunning = true;

            while(isRunning) {
                
                //Create an RX packet buffer
                packet = new DatagramPacket(rxBuffer, rxBuffer.length);
                
                //Wait for received data
                socket.receive(packet);
                Log.d(TAG, "Received UDP packet"); 
                
                //Add to received packet queue
                packetQueue.add(packet);                
            }
        } 
        catch (Throwable e) {
            
            Log.e(TAG, "UDP Client RX Exception");
            e.printStackTrace();
        } 
        finally {
            
            //Clear flag
            isRunning = false; 
            
            //Close the socket when thread exits
            if (socket != null) {
                
                socket.close();
            }
        }
    }
    
    /**
     * Send a UDP packet to the specified IP address and port
     * 
     * @param ip - destination IP address
     * @param port - destination port
     * @param msg - message to send
     * @param length - number of bytes to send
     * @return 1 if success, 0 otherwise
     */
    public int sendto(InetAddress ip, int port, byte[] msg, int length) {
        
        try {
                        
            DatagramPacket p = new DatagramPacket(msg, length, ip, port);           
            socket.send(p);
            Log.d(TAG, "Sent UDP packet");
            return 1;
        } 
        catch (Throwable e) {
            
            Log.e(TAG, "UDP Client TX Exception");
            e.printStackTrace();
        } 
                
        return 0;
    }
    
    /**
     * Send a message to the IP address and port set using the connect method
     * 
     * @param msg - message to send
     * @param length - number of bytes to send
     * @return 1 if success, 0 otherwise
     */
    public int send(byte[] msg, int length) {
                        
        if(isConnected) {
            
            return sendto(remoteIp, remotePort, msg, length);
        } 
                
        return 0;
    }
    
    /**
     * Get the next packet from the received message queue
     * 
     * @return Next UDP packet from the queue or null if queue is empty
     */
    public DatagramPacket recv() {
        
        if(!packetQueue.isEmpty()) {
                            
            return packetQueue.remove();
        }
                
        return null;
    }
    
    /**
     * Check if the receive queue has a message in it
     * 
     * @return true if queue has a message waiting in it, false if queue is empty
     */
    public boolean isRxDataReady() {
        
        return !packetQueue.isEmpty();
    }
    
    /**
     * Check if the receive thread is running
     * 
     * @return true if receive thread is running, false otherwise
     */
    public boolean isRunning() {
        
        return isRunning;
    }

    /**
     * Check if the destination IP address and port have been set using connect method 
     * 
     * @return true if destination IP address and port are set, false otherwise     
     */
    public boolean isConnected() {
        
        return isConnected;
    }
}
