
package com.example.ble_app;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Activity for scanning and displaying BLE devices that match our service UUID
 */
@TargetApi(Build.VERSION_CODES.M) // This is needed so that we can use Marshmallow API calls
public class MainActivity extends AppCompatActivity {

    private final static String TAG = MainActivity.class.getSimpleName();

    // This  tag that allows us to pass the address of the selected
    // BLE device to the control activity
    public static final String EXTRAS_BLE_ADDRESS = "BLE_ADDRESS";

    // BLE related objects
    private static BluetoothAdapter varBluetoothAdapter;
    private static BluetoothLeScanner varLEScanner;
    private static boolean varScanning;
    private static Handler varHandler;

    private static final int REQUEST_ENABLE_BLE = 1;
    // Scan for 10 seconds.
    private static final long SCAN_TIMEOUT = 10000;

    //This is required for Android 6.0 (Marshmallow)
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 1;

    // This allows rescanning when we swipe down from the top of the screen
    private static SwipeRefreshLayout mSwipeRefreshLayout;

    // This is the list view in the layout that holds the items
    ListView BleDeviceList;

    // These lists hold the BLE devices found during scanning and their names
    List<BluetoothDevice> varBluetoothDevice;
    List<String> varBleName;

    // The array adapter will be used to display the list of devices found during scanning
    ArrayAdapter<String> varBleArrayAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // This is the list view in the layout that holds the items
        BleDeviceList = (ListView) findViewById(R.id.BlelistItems);

        // This is used once scanning is started in a new thread
        varHandler = new Handler();

        // Check to see if the device supports BLE. If not, exit.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, R.string.no_ble, Toast.LENGTH_SHORT).show();
            finish();
        }

        // Initialize the Bluetooth adapter using the bluetoothManager
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        varBluetoothAdapter = bluetoothManager.getAdapter();

        // Check if Bluetooth manager returned the adapter. If not, exit.
        if (varBluetoothAdapter == null) {
            Toast.makeText(this, R.string.no_ble, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        //This section is required for Android 6.0 (Marshmallow) permissions
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // Android M Permission check 
            if (this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("This app needs location access ");
                builder.setMessage("Please grant location access so this app can detect devices.");
                builder.setPositiveButton(android.R.string.ok, null);
                builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
                    }
                });
                builder.show();
            }
        } //End of section for Android 6.0 (Marshmallow)
    }

    //This method is required for Android 6.0 (Marshmallow) permissions
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String permissions[], @NonNull int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_COARSE_LOCATION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Log.i("Permission for 6.0:", "Coarse location permission granted");
                } else {
                    final AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("Error");
                    builder.setMessage("Since location access has not been granted, scanning will not work.");
                    builder.setPositiveButton(android.R.string.ok, null);
                    builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                        }
                    });
                    builder.show();
                }
            }
        }
    } //End of section for Android 6.0 (Marshmallow)

    @Override
    protected void onResume() {
        super.onResume();

        // Verify that bluetooth is enabled. If not, request permission to enable it.
        if (!varBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BLE);

        }

        // Create arrays to hold BLE info found during scanning
        varBluetoothDevice = new ArrayList<>();
        varBleName = new ArrayList<>();
        // Create an array adapter and associate it with the list in the layout that displays the values
        varBleArrayAdapter = new ArrayAdapter<>(this, R.layout.ble_device_list, R.id.ble_name, varBleName);
        BleDeviceList.setAdapter(varBleArrayAdapter);
        // Setup the SwipeRefreshLayout and add a listener to refresh when the user
        // swipes down from the top of the screen.
        mSwipeRefreshLayout = (SwipeRefreshLayout) findViewById(R.id.swipeRefreshId);

        // Setup a listener for swipe events
        mSwipeRefreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                if (!varScanning) {
                    varBluetoothDevice.clear(); // Remove all existing devices
                    varBleArrayAdapter.clear();
                    scanLeDevice(true); // Start a scan if not already running
                    Log.i(TAG, "Rescanning");
                }
                mSwipeRefreshLayout.setRefreshing(false);
            }
        });

        // Set up a listener for when the user clicks on one of the devices found.
        // We need to launch the display activity when that happens
        BleDeviceList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Log.i(TAG, "Item Selected");
                final Intent intent = new Intent(MainActivity.this, DisplayActivity.class);
                // Send the address of the device that was selected so that the display activity
                // knows which device to connect with
                intent.putExtra(EXTRAS_BLE_ADDRESS, varBluetoothDevice.get(position).getAddress());
                scanLeDevice(false); // Stop scanning
                startActivity(intent);
            }
        });

        scanLeDevice(true); // Start scanning automatically when we first start up
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // User chose not to enable Bluetooth so exit.
        if (requestCode == REQUEST_ENABLE_BLE && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onPause() {
        super.onPause();
        scanLeDevice(false);
        varBluetoothDevice.clear();
        varBleArrayAdapter.clear();
        mSwipeRefreshLayout.setRefreshing(false);
    }

    /**
     * Start or stop BLE scanning
     *
     * @param enable start scanning if true
     */
    private void scanLeDevice(final boolean enable) {
        if (enable) { // enable set to start scanning
            // Stops scanning after a pre-defined scan period.
            varHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    if(varScanning) {
                        varScanning = false;
                        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                            //noinspection deprecation
                            varBluetoothAdapter.stopLeScan(mLeScanCallback);
                        } else {
                            varLEScanner.stopScan(mScanCallback);
                        }
                        invalidateOptionsMenu();
                    }
                }
            }, SCAN_TIMEOUT);

            varScanning = true;
            UUID[] motorServiceArray = {BLEService.getDROServiceUUID()};
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                //noinspection deprecation
                varBluetoothAdapter.startLeScan(motorServiceArray, mLeScanCallback);
            } else { // New BLE scanning introduced in LOLLIPOP
                ScanSettings settings;
                List<ScanFilter> filters;
                varLEScanner = varBluetoothAdapter.getBluetoothLeScanner();
                settings = new ScanSettings.Builder()
                        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                        .build();
                filters = new ArrayList<>();
                // Scan just for the PSoC 6 DRO's UUID
                ParcelUuid PUuid = new ParcelUuid(BLEService.getDROServiceUUID());
                ScanFilter filter = new ScanFilter.Builder().setServiceUuid(PUuid).build();
                filters.add(filter);
                // Use this code to add filter on device address.
//                private static String varDeviceAddress = "00:A0:50:00:00:00";
//                ScanFilter filter = new ScanFilter.Builder().setDeviceAddress(varDeviceAddress).build();
//                filters.add(filter);
                varLEScanner.startScan(filters, settings, mScanCallback);
            }
        } else { // enable set to stop scanning
            if(varScanning) {
                varScanning = false;
                if (Build.VERSION.SDK_INT < 21) {
                    //noinspection deprecation
                    varBluetoothAdapter.stopLeScan(mLeScanCallback);
                } else {
                    varLEScanner.stopScan(mScanCallback);
                }
            }
        }
        invalidateOptionsMenu();
    }

    /**
     * This is the callback for BLE scanning on versions prior to LOLLIPOP
     * It is called each time a device is found so we need to add it to the list
     */
    private final BluetoothAdapter.LeScanCallback mLeScanCallback = new BluetoothAdapter.LeScanCallback() {
        @Override
        public void onLeScan(final BluetoothDevice device, int rssi, byte[] scanRecord) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(!varBluetoothDevice.contains(device)) { // only add new devices
                        varBluetoothDevice.add(device);
                        varBleName.add(device.getName());
                        varBleArrayAdapter.notifyDataSetChanged(); // Update the list on the screen
                    }

                }
            });
        }
    };

    /**
     * This is the callback for BLE scanning for LOLLIPOP and later
     * It is called each time a device is found so we need to add it to the list
     */
    private final ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if(!varBluetoothDevice.contains(result.getDevice())) { // only add new devices
                varBluetoothDevice.add(result.getDevice());
                varBleName.add(result.getDevice().getName());
                varBleArrayAdapter.notifyDataSetChanged(); // Update the list on the screen
            }

        }
    };
}
