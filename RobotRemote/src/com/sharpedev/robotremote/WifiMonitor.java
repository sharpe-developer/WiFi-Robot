/******************************************************************************
 * NAME: WifiMonitor
 * 
 * DESCRIPTION:
 *   Class for monitoring connection status for a specified WiFi network.
 *   Provides an option to automatically attempt to connect to the specified
 *   network if not connected.
 *****************************************************************************/
package com.sharpedev.robotremote;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

public class WifiMonitor extends BroadcastReceiver {
        
    //Tag for log messages
    private final String TAG = this.getClass().getSimpleName();
    
    String              networkName         = "";
    String              password            = "";       
    WifiConfiguration   wifiConfig          = null;
    WifiManager         wifiManager         = null;
    ConnectivityManager connectivityManager = null;
    Thread              autoConnectThread   = null;
    int                 netId               = 0;      
    boolean             isConnected         = false;
    

    
    /**
     * Class Constructor
     * 
     * @param context - application context (needed to retrieve services and register broadcast receiver)
     */
    public WifiMonitor(Context context) {
            
        //Get necessary objects for this class to function
        wifiConfig = new WifiConfiguration();
        wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);        
        connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        
        //Start with default network credentials
        setNetwork(networkName, password);
        
        //Register broadcast receiver to catch WIFI connectivity change events
        context.registerReceiver(this, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
    }   

    /**
     * Set the monitored network parameters (SSID and password) and 
     * update the connection status. 
     * 
     * Note: Assumes WPA for secure networks (i.e. password not blank)
     * 
     * @param name - the network SSID string
     * @param pwd - the network password string (empty string if unsecured)
     */
    public void setNetwork(String name, String pwd) {
        
        networkName = name;
        password = pwd;
        
        //Set network access point name
        wifiConfig.SSID = "\"" + networkName + "\"";
                
        //Assumes WPA if password is used
        if(password.length() > 0)
        {
            wifiConfig.preSharedKey = "\"" + password + "\"";
        }
        else //open network
        {
            wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            wifiConfig.preSharedKey = null;
        }
        
        //Check connection status
        isWifiConnected();
    }
    
    /**
     * Get the network SSID for the network being monitored
     * 
     * @return network SSID
     */
    public String getNetworkName() {
        
        return networkName;
    }
    
    /**
     * Get the password for the network being monitored
     * 
     * @return network password
     */
    public String getPassword() {
        
        return password;
    }
    
    /**
     * Run a thread that will monitor the WIFI state and automatically 
     * attempt to connect to the specified WIFI when not connected
     */
    public void enableAutoConnect() {
                       
        runAutoConnectThread();
    }
    
    /**
     * Stop the auto connect thread
     */
    public void disableAutoConnect() {
        
        //interrupt the auto connect thread to stop it
        autoConnectThread.interrupt();
    }   
    
    /**
     * Thread that monitors the WIFI connection status and automatically 
     * attempts to connect to the specified WIFI when not connected
     */
    public void runAutoConnectThread() {                                                  
      
        if(autoConnectThread == null) {
        
            //Start a thread to connect to desired WIFI when connection is not active
            autoConnectThread = new Thread() {
                
                @Override
                public void run() {
                    try 
                    {
                        Log.d(TAG, "Starting WIFI auto connect thread loop");                   
                        
                        //thread loop
                        while (!autoConnectThread.isInterrupted()) 
                        {     
                            //If not connected to the desired WIFI
                            if(!isConnected) 
                            {
                                //Try to connect to the desired WIFI network
                                Log.d(TAG, "Attempting to connect to WiFi"); 
                                connectToWifi();
                                
                                //Allow some extra time for connection to establish
                                Thread.sleep(4000); 
                            }
                            
                            //Sleep a while
                            Thread.sleep(1000);                         
                        }                                        
                    } 
                    catch(InterruptedException e) {
                        
                        //Mark the thread as inactive
                        autoConnectThread = null;
                        return;
                    }
                    catch (Exception e) {
                    
                        Log.e(TAG, "WIFI auto connect thread loop exception");
                        e.printStackTrace();
                    }
                }
            };
            autoConnectThread.start(); 
        }
    }    
    
    /**
     * Get the connected flag indicating if the device is currently 
     * connected to the specified network
     * 
     * @return true if connected, false otherwise
     */
    public boolean isConnected() {      
        
        return isConnected;
    }
    
    /**
     * Checks if the device is currently connected to the specified network
     * and sets the connected flag accordingly
     * 
     * @return true if connected, false otherwise
     */
    public boolean isWifiConnected() {              
                
        //Get network info from connectivity manager
        NetworkInfo netInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        
        Log.i(TAG, "WIFI State: " + netInfo.getState().toString());
        
        //Check connectivity manager connection status      
        if(!netInfo.isConnected())
        {
            isConnected = false;
            return isConnected;  
        }
        
        //Get WifiManager connection info
        WifiInfo info = wifiManager.getConnectionInfo();        
        
        //If SSID is available
        if(info.getSSID() == null)
        {
            isConnected = false;
            return isConnected;
        }
        
        Log.i(TAG, "Connected to: " + info.getSSID());
        
        //Are we connected to the network with the SSID we want?
        if(info.getSSID().matches(networkName))
        {               
            isConnected = true;
        }
        else
        {
            isConnected = false;            
        }
        
        return isConnected;
    }
    
    /**
     * Attempt to establish a connection to the specified network
     */
    public void connectToWifi() {           

        //Is WIFI disabled?
        if (!wifiManager.isWifiEnabled()) 
        { 
            //WIFI is turned off, turn on WIFI
            wifiManager.setWifiEnabled(true);
        }    
        
        //Check if network is already listed in WifiManager before adding it
        boolean found = false;
        
        for(WifiConfiguration cfg : wifiManager.getConfiguredNetworks()) {
            if(cfg.SSID.equals(wifiConfig.SSID)) {
                                
                //If already connected to the desired network
                if(isWifiConnected() == true) {
                    
                    Log.d(TAG, "Already connected to desired WIFI");
                    return;
                }
                
                //The SSID matches
                //If the password in WIFI manager is different than the stored network credentials
                if(cfg.preSharedKey == null && password.length() > 0 || 
                   cfg.preSharedKey != null && !cfg.preSharedKey.equals(password)) {
                    
                    //Remove the network from WIFI manager and allow it to 
                    //be added again using the new password
                    wifiManager.removeNetwork(cfg.networkId);
                }
                else {
                    
                    //Record the network ID for call to enable network and 
                    //set flag that the desired network was found
                    netId = cfg.networkId; 
                    found = true;   
                    break;
                }
                
                Log.d(TAG, "Found desired SSID in Wifi Manager");
            }
        }
        
        //If the network is not already listed with the WifiManager
        if(!found) {
            
            //Add the desired network to the manager
            netId = wifiManager.addNetwork(wifiConfig);
        }
        
        //Connect to network, disabling any currently connected
        wifiManager.enableNetwork(netId, true);  
    }
    
    /**
     * Broadcast receiver for connectivity change events
     * 
     * @see android.content.BroadcastReceiver#onReceive(android.content.Context, android.content.Intent)
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        
        //WIFI connectivity event has occurred. Check the connection status.
        isWifiConnected();      
    }
}