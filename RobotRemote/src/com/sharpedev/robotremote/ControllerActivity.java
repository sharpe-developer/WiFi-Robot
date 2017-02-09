/******************************************************************************
 * NAME: ControllerActivity
 * 
 * DESCRIPTION:
 *   Main user interface for sending commands and displaying status from the 
 *   device under control. 
 *****************************************************************************/
package com.sharpedev.robotremote;


import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.app.AlertDialog;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageButton;

public class ControllerActivity extends Activity {  
    
    RobotRemoteApp  app           = null;
    AlertDialog     alertDialog   = null;
    ImageButton     forwardButton = null;
    ImageButton     backButton    = null;
    ImageButton     rightButton   = null;
    ImageButton     leftButton    = null;
    Thread          monitorThread = null;
    
    //Constants for UI messages sent from threads to UI thread
    private class UiMsg {
        public final static int DISMISS_ALERT = 0;
        public final static int SHOW_ALERT    = 1;
    }
    
    //Inner class to process UI messaging from non-UI threads to the UI thread
    Handler uiMsgHandler = new Handler () {
        
        public void handleMessage (Message msg) {

            switch(msg.what) {
                case UiMsg.DISMISS_ALERT:
                    alertDialog.dismiss();
                    break;
                case UiMsg.SHOW_ALERT:
                    alertDialog.show();
                    break;
            }         
        }
    };
    
    /**
     * Create the activity. Get the context of the application 
     * object which will be responsible for keeping the 
     * interface for the device under control persistent 
     * through activity starts and stops.
     * 
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        //Record the application class. This is the owner of the UDP interface.
        //Commands will be sent through the application class
        app = (RobotRemoteApp)getApplication();     
        
        //Create a dialog to alert the user when connection with robot is lost
        alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle("Robot Remote");
        alertDialog.setMessage("Waiting for robot network connection...");
        
        //Get the button objects
        forwardButton = (ImageButton)findViewById(R.id.buttonFwd);
        backButton    = (ImageButton)findViewById(R.id.buttonBack);
        rightButton   = (ImageButton)findViewById(R.id.buttonRight);
        leftButton    = (ImageButton)findViewById(R.id.buttonLeft);
        
        //Inner class for common button touch handling code
        class ButtonEventHandler implements OnTouchListener {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                
                //Assume we do not handle the event
                boolean ret = false;
                
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    
                    //Determine which button was pressed and 
                    //send that direction command to the robot
                    //TODO allow user to control speed value in sendCommand method call
                    switch (view.getId()) {
                        case R.id.buttonFwd:
                            app.sendCommand(Directions.FORWARD, 100);
                            ret = true;
                            break;
                        case R.id.buttonBack:
                            app.sendCommand(Directions.BACKWARD, 100);
                            ret = true;
                            break;
                        case R.id.buttonRight:
                            app.sendCommand(Directions.RIGHT, 100); 
                            ret = true;
                            break;
                        case R.id.buttonLeft:
                            app.sendCommand(Directions.LEFT, 100); 
                            ret = true;
                            break;
                        default: 
                            break;
                    }
                } 
                else if (event.getAction() == MotionEvent.ACTION_UP) {
                    
                    //All up events send stop command to the robot
                    app.sendCommand(Directions.STOP, 0);
                    ret = true;
                }
                
                return ret;
            }
        }
                
        //Set listeners for the robot movement buttons
        forwardButton.setOnTouchListener(new ButtonEventHandler());
        backButton.setOnTouchListener(new ButtonEventHandler());
        rightButton.setOnTouchListener(new ButtonEventHandler());
        leftButton.setOnTouchListener(new ButtonEventHandler());                    
    }
    
    /**
     * @see android.app.Activity#onResume()
     */
    @Override
    protected void onResume() {
        super.onResume();
        // The activity has become visible
        
        //If monitor thread is not already running 
        if(monitorThread == null) {
            
            //Start a thread to monitor for the robot connection
            runConnectionMonitor(); 
        }
    }
    
    /**
     * @see android.app.Activity#onPause()
     */
    @Override
    protected void onPause() {
        super.onPause();
        
        // Another activity is taking focus. Stop the monitor thread
        monitorThread.interrupt();
    }
    
    /* (non-Javadoc)
     * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }   
    
    /**
     * Run a thread that checks if the connection to the device is still active.
     * If it is not then the activity displays a message to alert the user that 
     * communications with the controlled device have been lost.
     */
    public void runConnectionMonitor() {                                        
        
        //Start a thread to monitor for the robots connection status
        monitorThread = new Thread() {
            @Override
            public void run() {             
                try {                   
                    
                    while(!monitorThread.isInterrupted()) {
                        
                        if(!app.isRobotConnected()) {
                            
                            Log.i("RobotRemote", "Activity waiting for robot to connect");
                                                    
                            //Show a dialog until the app is connected to the robot
                            uiMsgHandler.sendEmptyMessage(UiMsg.SHOW_ALERT); 
                            
                            //check if connected!
                            while (!app.isRobotConnected()) { 
                                
                                //Wait for connect
                                Thread.sleep(1000);                     
                            }
                            
                            //Robot is connected, dismiss the alert dialog
                            uiMsgHandler.sendEmptyMessage(UiMsg.DISMISS_ALERT);
                            
                            Log.i("RobotRemote", "Activity ready"); 
                        }
                        
                        Thread.sleep(1000);
                    }                                        
                } 
                catch(InterruptedException e) {
                    
                    monitorThread = null;
                    return;
                }
                catch (Exception e) {
                    
                    Log.e("RobotRemote", "Activity connect monitor exception");
                    e.printStackTrace();
                }                
            }
        };
        
        monitorThread.start();               
    }
}

