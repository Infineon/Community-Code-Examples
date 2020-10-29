package com.example.ble_app;

import androidx.appcompat.app.AppCompatActivity;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Locale;

public class DisplayActivity extends AppCompatActivity {

    // This tag is used for debug messages
    private static final String TAG = DisplayActivity.class.getSimpleName();

    // Variables used for shared preferences
    private static final String SHARED_PREFS = "sharedPrefs";
    private static final String XFULL_LENGTH = "xFullLength";
    private static final String YFULL_LENGTH = "yFullLength";
    private static final String ZFULL_LENGTH = "zFullLength";
    private static final String XFULL_COUNT = "xFullCount";
    private static final String YFULL_COUNT = "yFullCount";
    private static final String ZFULL_COUNT = "zFullCount";

    // Index variables to improve code readability
    private static final int XLENGTH_INDEX = 0;
    private static final int XCOUNT_INDEX = 1;
    private static final int YLENGTH_INDEX = 2;
    private static final int YCOUNT_INDEX = 3;
    private static final int ZLENGTH_INDEX = 4;
    private static final int ZCOUNT_INDEX = 5;

    private static final int XAXIS_INDEX = 0;
    private static final int YAXIS_INDEX = 1;
    private static final int ZAXIS_INDEX = 2;


    //Variable to hold the user data to be saved
    private static int[] varSaveData = new int[6];
    //Variable to hold the encoder readings that are received
    private static float[] varAxisData = new float[3];


    private static String varDeviceAddress;
    private static BLEService varBLEService;

    //Variables for the UI elements
    private static Button varButtonSetMachineParam;
    private static Button varButtonSave;
    private static Button varButtonSetWorkspace0;

    private static EditText varEditTextNumberX;
    private static EditText varEditTextNumberY;
    private static EditText varEditTextNumberZ;

    private static EditText varEditTextNumberXlength;
    private static EditText varEditTextNumberXCount;
    private static EditText varEditTextNumberYlength;
    private static EditText varEditTextNumberYCount;
    private static EditText varEditTextNumberZlength;
    private static EditText varEditTextNumberZCount;

    private static TableLayout varTableLayoutMachineParam;

    /**
     * This manages the lifecycle of the BLE service.
     * When the service starts we get the service object, initialize the service, and connect.
     */
    private final ServiceConnection varServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            Log.i(TAG, "onServiceConnected");
            varBLEService = ((BLEService.LocalBinder) service).getService();
            if (!varBLEService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }
            // Automatically connects to the PSoC 6 DRO database upon successful start-up initialization.
            varBLEService.connect(varDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            varBLEService = null;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_display);

        // Assign the various layout objects to the appropriate variables
        varButtonSetMachineParam = (Button) findViewById(R.id.buttonsetmachparam);
        varButtonSave = (Button) findViewById(R.id.buttonsave);
        varButtonSetWorkspace0 = (Button) findViewById(R.id.buttonSetWorkspace0);

        varEditTextNumberX = (EditText) findViewById(R.id.editTextNumberX);
        varEditTextNumberY = (EditText) findViewById(R.id.editTextNumberY);
        varEditTextNumberZ = (EditText) findViewById(R.id.editTextNumberZ);

        varEditTextNumberXlength = (EditText) findViewById(R.id.editTextNumberXLength);
        varEditTextNumberXCount = (EditText) findViewById(R.id.editTextNumberXCount);
        varEditTextNumberYlength = (EditText) findViewById(R.id.editTextNumberYLength);
        varEditTextNumberYCount = (EditText) findViewById(R.id.editTextNumberYCount);
        varEditTextNumberZlength = (EditText) findViewById(R.id.editTextNumberZLength);
        varEditTextNumberZCount = (EditText) findViewById(R.id.editTextNumberZCount);

        varTableLayoutMachineParam = (TableLayout) findViewById(R.id.tableLayoutMachineParam);

        final Intent intent = getIntent();
        varDeviceAddress = intent.getStringExtra(MainActivity.EXTRAS_BLE_ADDRESS);

        // Bind to the BLE service
        Log.i(TAG, "Binding Service");
        Intent DROServiceIntent = new Intent(this, BLEService.class);
        bindService(DROServiceIntent, varServiceConnection, BIND_AUTO_CREATE);

        // Get previously saved user data
        loadMachineParam();

        // This will be called when the Set Machine Parameters button is pressed
        varButtonSetMachineParam.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                machineSetParam();
            }
        });

        // Listener to save the user data
        varButtonSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                saveParam();
            }
        });

        // Listener to send workspace 0 to PSoC 6 DRO
        varButtonSetWorkspace0.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                BLEService.sendResetCommand((char) 0x01);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(DROUpdateReceiver, makeDROUpdateIntentFilter());
        if (varBLEService != null) {
            final boolean result = varBLEService.connect(varDeviceAddress);
            Log.i(TAG, "Connect request result=" + result);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterReceiver(DROUpdateReceiver);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(varServiceConnection);
        varBLEService = null;
    }

    /**
     * Function to load the previously saved user data
     */
    private void loadMachineParam() {
        SharedPreferences sharedPreferences = getSharedPreferences(SHARED_PREFS, MODE_PRIVATE);
        varSaveData[XLENGTH_INDEX] = sharedPreferences.getInt(XFULL_LENGTH, 1000);
        varSaveData[XCOUNT_INDEX] = sharedPreferences.getInt(XFULL_COUNT, 360000);
        varSaveData[YLENGTH_INDEX] = sharedPreferences.getInt(YFULL_LENGTH, 1000);
        varSaveData[YCOUNT_INDEX] = sharedPreferences.getInt(YFULL_COUNT, 360000);
        varSaveData[ZLENGTH_INDEX] = sharedPreferences.getInt(ZFULL_LENGTH, 1000);
        varSaveData[ZCOUNT_INDEX] = sharedPreferences.getInt(ZFULL_COUNT, 360000);
    }

    /**
     * Function to display the user data in the respective UI elements
     */
    private void displayMachineParam() {
        varEditTextNumberXlength.setText(String.format(Locale.getDefault(), "%d", varSaveData[XLENGTH_INDEX]), TextView.BufferType.NORMAL);
        varEditTextNumberXCount.setText(String.format(Locale.getDefault(), "%d", varSaveData[XCOUNT_INDEX]), TextView.BufferType.NORMAL);
        varEditTextNumberYlength.setText(String.format(Locale.getDefault(), "%d", varSaveData[YLENGTH_INDEX]), TextView.BufferType.NORMAL);
        varEditTextNumberYCount.setText(String.format(Locale.getDefault(), "%d", varSaveData[YCOUNT_INDEX]), TextView.BufferType.NORMAL);
        varEditTextNumberZlength.setText(String.format(Locale.getDefault(), "%d", varSaveData[ZLENGTH_INDEX]), TextView.BufferType.NORMAL);
        varEditTextNumberZCount.setText(String.format(Locale.getDefault(), "%d", varSaveData[ZCOUNT_INDEX]), TextView.BufferType.NORMAL);
    }

    /**
     * Enable the user to set full scale count and full scale length values
     */
    private void machineSetParam() {
        varTableLayoutMachineParam.setVisibility(View.VISIBLE);
        varButtonSave.setVisibility(View.VISIBLE);
        loadMachineParam();
        displayMachineParam();
    }

    /**
     * Check if the user data provided is valid
     *
     * @return Return true if the user data is valid
     */
    private boolean checkMachineParam() {
        try {
            varSaveData[XLENGTH_INDEX] = Integer.parseInt(varEditTextNumberXlength.getText().toString());
            varSaveData[XCOUNT_INDEX] = Integer.parseInt(varEditTextNumberXCount.getText().toString());
            varSaveData[YLENGTH_INDEX] = Integer.parseInt(varEditTextNumberYlength.getText().toString());
            varSaveData[YCOUNT_INDEX] = Integer.parseInt(varEditTextNumberYCount.getText().toString());
            varSaveData[ZLENGTH_INDEX] = Integer.parseInt(varEditTextNumberZlength.getText().toString());
            varSaveData[ZCOUNT_INDEX] = Integer.parseInt(varEditTextNumberZCount.getText().toString());
        } catch (Exception e)
        {
            Toast.makeText(DisplayActivity.this, "Invalid Machine Parameters\nData not saved", Toast.LENGTH_LONG).show();
            return false;
        }

        for(int i = 0; i<6; i++)
        {
            if(varSaveData[i] == 0) {
                Toast.makeText(DisplayActivity.this, "Invalid Machine Parameters\nData not saved", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        return true;
    }

    /**
     * Save the user data using shared preferences
     */
    private void saveParam() {
        SharedPreferences sharedPreferences = getSharedPreferences(SHARED_PREFS, MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();

        if(checkMachineParam()) {
            editor.putInt(XFULL_LENGTH, varSaveData[XLENGTH_INDEX]);
            editor.putInt(XFULL_COUNT, varSaveData[XCOUNT_INDEX]);
            editor.putInt(YFULL_LENGTH, varSaveData[YLENGTH_INDEX]);
            editor.putInt(YFULL_COUNT, varSaveData[YCOUNT_INDEX]);
            editor.putInt(ZFULL_LENGTH, varSaveData[ZLENGTH_INDEX]);
            editor.putInt(ZFULL_COUNT, varSaveData[ZCOUNT_INDEX]);

            editor.apply();
            Toast.makeText(DisplayActivity.this, "Data Saved Successfully!", Toast.LENGTH_LONG).show();

            varTableLayoutMachineParam.setVisibility(View.INVISIBLE);
            varButtonSave.setVisibility(View.INVISIBLE);
        }
    }

    /**
     * Function to calculate the axis values based on the received encoder readings and saved user data
     */
    private void calculateAxisValues() {
        varAxisData[XAXIS_INDEX] = ((float) BLEService.getEncoderValue(BLEService.Encoder.ENCODER_X) / (float) varSaveData[XCOUNT_INDEX]) * (float) varSaveData[XLENGTH_INDEX];
        varAxisData[YAXIS_INDEX] = ((float) BLEService.getEncoderValue(BLEService.Encoder.ENCODER_Y) / (float) varSaveData[YCOUNT_INDEX]) * (float) varSaveData[YLENGTH_INDEX];
        varAxisData[ZAXIS_INDEX] = ((float) BLEService.getEncoderValue(BLEService.Encoder.ENCODER_Z) / (float) varSaveData[ZCOUNT_INDEX]) * (float) varSaveData[ZLENGTH_INDEX];
    }

    /**
     * Function to display the axis values in the respective UI elements
     */
    private void displayAxisValues() {
        varEditTextNumberX.setText(String.format(Locale.getDefault(), "%.3f", varAxisData[XAXIS_INDEX]) + " mm", TextView.BufferType.NORMAL);
        varEditTextNumberY.setText(String.format(Locale.getDefault(), "%.3f", varAxisData[YAXIS_INDEX]) + " mm", TextView.BufferType.NORMAL);
        varEditTextNumberZ.setText(String.format(Locale.getDefault(), "%.3f", varAxisData[ZAXIS_INDEX]) + " mm", TextView.BufferType.NORMAL);
    }

    /**
     * Handle broadcasts from the PSoC 6 DRO service object. The events are:
     * ACTION_CONNECTED: connected to the car.
     * ACTION_DISCONNECTED: disconnected from the car.
     * ACTION_DATA_AVAILABLE: received data from the car.  This can be a result of a read
     * or notify operation.
     */
    private final BroadcastReceiver DROUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            switch (action) {
                case BLEService.ACTION_CONNECTED:
                    // No need to do anything here. Service discovery is started by the service.
                    break;
                case BLEService.ACTION_DISCONNECTED:
                    varBLEService.close();
                    break;
                case BLEService.ACTION_DATA_AVAILABLE:
                    // This is called after a Notify completes
                    // Calculate the axis values based on the user data saved
                    calculateAxisValues();
                    // Display the calculated axis values
                    displayAxisValues();
                    break;
            }
        }
    };

    /**
     * This sets up the filter for broadcasts that we want to be notified of.
     * This needs to match the broadcast receiver cases.
     *
     * @return intentFilter
     */
    private static IntentFilter makeDROUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BLEService.ACTION_CONNECTED);
        intentFilter.addAction(BLEService.ACTION_DISCONNECTED);
        intentFilter.addAction(BLEService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }
}