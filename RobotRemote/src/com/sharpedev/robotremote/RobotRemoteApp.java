/******************************************************************************
 * NAME: RobotRemoteApp
 * 
 * DESCRIPTION:
 *   Main application object. This is utilized to allow the interface  
 *   to the device under control to persist through various activity starts 
 *   and stops. For example we do not want to disconnect and reconnect to WIFI
 *   every time the controller activity loses focus.
 *****************************************************************************/
package com.sharpedev.robotremote;

import java.net.DatagramPacket;
import java.net.InetAddress;

import android.app.Application;
import android.util.Log;

/**
 * Enumeration of robot direction commands
 */
enum Directions
{
  STOP,          
  FORWARD,       
  BACKWARD, 
  LEFT,
  SHARP_LEFT,    
  RIGHT,         
  SHARP_RIGHT        
};

public class RobotRemoteApp extends Application {
    
    UdpSocket   udp  = null;
    WifiMonitor wifi = null;
    
    //TODO Allow user to set these parameters from an activity
    String robotSsid = "STM8S_Robot";
    String robotPwd  = "";
    String robotIp   = "192.168.4.1";
    int    robotPort = 49999;
    
    /**
     * Create the WIFI monitor the connection to the robot and prepare
     * a UDP socket for communicating with the robot. Start a thread 
     * to monitor for a valid connection and open the UDP socket once 
     * we are connected to the robot's WIFI network access point.  
     * 
     * @see android.app.Application#onCreate()
     */
    @Override
    public void onCreate() {
        super.onCreate();
        try {
                        
            wifi = new WifiMonitor(this);  
            udp  = new UdpSocket(robotPort); //Robot and remote use same port
            
            //Set the network credentials for the network to be monitored
            wifi.setNetwork(robotSsid, robotPwd);
            wifi.enableAutoConnect();
            
            //Set robot address and port number
            udp.connect(InetAddress.getByName(robotIp), robotPort);
            
            //Run application thread
            runAppThread();   
        }
        catch (Exception e) {
            
            Log.e("RobotRemote", "App Create Exception");           
        }
    }    
    
    /**
     * Thread to monitor for a valid connection to the robot's WIFI network 
     * access point and monitor for received messages from the UDP socket 
     */
    public void runAppThread() {         
       
        //Start a thread to process events for the app
        Thread t = new Thread() {
            @Override
            public void run() {
                try 
                {    
                    while(true)
                    {
                        //Wait for connection to the robots WIFI access point 
/*                      if (!wifi.isConnected()) 
                        {         
                            //Lost WIFI connection. Stop UDP client.
                            udp.stop();
                        }                                             
*/                                              
                        //If WIFI is connected but UDP socket is not running (i.e. bound) then start it
                        if(wifi.isConnected() && !udp.isRunning())
                        {
                            udp.start();
                        }
                        
                        //Check if received data from the robot is available
                        if(udp.isRxDataReady()) {
                            
                            DatagramPacket packet = udp.recv();
                            
                            //TODO process received packets
                            Log.d("RobotRemote", "RX packet from " + packet.getAddress() + ":" + packet.getPort());
                                                        
                        }
                        
                        Thread.sleep(1000);
                    }                           
                } 
                catch (Exception e) {
                    
                    Log.e("RobotRemote", "App Monitor Thread Exception");
                    e.printStackTrace();
                }
            }
        };
        t.start();
    }
    
    /**
     * Send a movement command message to the robot
     * 
     * @param cmd - the direction the robot should move
     * @param speed - the speed the robot should move (0% to 100%)
     */
    public void sendCommand(Directions cmd, int speed) {
        
        //Robot command message structure constants
        final int DIRECTION     = 0;
        final int SPEED         = 1;
        final int NUM_MSG_BYTES = 2;
        
        //Create message buffer
        byte[] msg = new byte[NUM_MSG_BYTES];
        
        //Set direction
        msg[DIRECTION] = (byte) cmd.ordinal();
        
        //Set speed
        msg[SPEED] = (byte) speed;          
                
        //Send message to robot
        udp.send(msg, msg.length);      
    }
    
    /**
     * Check if the robot communication interface is currently connected.
     * 
     * @return true if robot interface is connected, false otherwise
     */
    public boolean isRobotConnected() {
        
        boolean isConnected = false;
        
        if(wifi != null)
        {
            isConnected = wifi.isConnected();
        }
        
        return isConnected;
    }
}

