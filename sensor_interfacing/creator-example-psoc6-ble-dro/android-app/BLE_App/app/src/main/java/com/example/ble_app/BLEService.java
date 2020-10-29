
package com.example.ble_app;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import java.util.LinkedList;
import java.util.Queue;
import java.util.UUID;

/**
 * Service for managing connection and data communication with the PSoC 6 DRO
 */
public class BLEService extends Service {

    private final static String TAG = BLEService.class.getSimpleName();

    public enum Encoder { ENCODER_X, ENCODER_Y, ENCODER_Z }

    private static BluetoothManager varBluetoothManager;
    private static BluetoothAdapter varBluetoothAdapter;
    private static String varBluetoothDeviceAddress;
    private static BluetoothGatt varBluetoothGatt;

    //  Queue for BLE events
    //  This is needed so that rapid BLE events don't get dropped
    private static final Queue<Object> BleQueue = new LinkedList<>();

    // UUID for the custom DRO characteristics
    private static final String baseUUID =           "00000000-0000-1000-8000-00805f9b34f";
    private static final String DROServiceUUID =   baseUUID + "0";
    private static final String encoderXCharUUID =  baseUUID + "1";
    private static final String encoderYCharUUID = baseUUID + "2";
    private static final String encoderZCharUUID =   baseUUID + "3";
    private static final String workspace0CharUUID = baseUUID + "4";

    private static final String CCCD_UUID =          "00002902-0000-1000-8000-00805f9b34fb";

    // Bluetooth Characteristics that we need to read/write
    private static BluetoothGattCharacteristic encoderXCharacteristic;
    private static BluetoothGattCharacteristic encoderYCharacteristic;
    private static BluetoothGattCharacteristic encoderZCharacteristic;
    private static BluetoothGattCharacteristic workspace0Characteristic;

    // Variables to save encoder values
    private static int encoderXValue;
    private static int encoderYValue;
    private static int encoderZValue;

    // Actions used during broadcasts to the activity
    public static final String ACTION_CONNECTED =
            "com.example.ble_app.ACTION_GATT_CONNECTED";
    public static final String ACTION_DISCONNECTED =
            "com.example.ble_app.ACTION_GATT_DISCONNECTED";
    public static final String ACTION_DATA_AVAILABLE =
            "com.example.ble_app.ACTION_DATA_AVAILABLE";

    /**
     * This is a binder for the BluetoothLeService
     */
    public class LocalBinder extends Binder {
        BLEService getService() {
            return BLEService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return varBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // Disconnect from the GATT database and close the connection
        disconnect();
        close();
        return super.onUnbind(intent);
    }

    private final IBinder varBinder = new LocalBinder();

    /**
     * Implements callback methods for GATT events.
     */
    private final BluetoothGattCallback varGattCallback = new BluetoothGattCallback() {
        /**
         * This is called on a connection state change (either connection or disconnection)
         * @param gatt The GATT database object
         * @param status Status of the event
         * @param newState New state (connected or disconnected)
         */
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                broadcastUpdate(ACTION_CONNECTED);
                Log.i(TAG, "Connected to GATT server.");
                // Attempts to discover services after successful connection.
                Log.i(TAG, "Attempting to start service discovery:" +
                        varBluetoothGatt.discoverServices());

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "Disconnected from GATT server.");
                broadcastUpdate(ACTION_DISCONNECTED);
            }
        }

        /**
         * This is called when service discovery has completed.
         *
         * It broadcasts an update to the main activity.
         *
         * @param gatt The GATT database object
         * @param status Status of whether the discovery was successful.
         */
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                // Get the characteristics for the DRO service
                BluetoothGattService gattService = varBluetoothGatt.getService(UUID.fromString(DROServiceUUID));
                if (gattService == null) return; // return if the DRO service is not supported
                encoderXCharacteristic = gattService.getCharacteristic(UUID.fromString(encoderXCharUUID));
                encoderYCharacteristic = gattService.getCharacteristic(UUID.fromString(encoderYCharUUID));
                encoderZCharacteristic = gattService.getCharacteristic(UUID.fromString(encoderZCharUUID));
                workspace0Characteristic = gattService.getCharacteristic(UUID.fromString(workspace0CharUUID));

                // Set the CCCD to notify us for the three encoder readings
                setCharacteristicNotification(encoderXCharacteristic, true);
                setCharacteristicNotification(encoderYCharacteristic, true);
                setCharacteristicNotification(encoderZCharacteristic, true);

            } else {
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        /**
         * This handles the BLE Queue. If the queue is not empty, it starts the next event.
         */
        private void handleBleQueue() {
            if(BleQueue.size() > 0) {
                // Determine which type of event is next and fire it off
                if (BleQueue.element() instanceof BluetoothGattDescriptor) {
                    varBluetoothGatt.writeDescriptor((BluetoothGattDescriptor) BleQueue.element());
                } else if (BleQueue.element() instanceof BluetoothGattCharacteristic) {
                    varBluetoothGatt.writeCharacteristic((BluetoothGattCharacteristic) BleQueue.element());
                }
            }
        }

        /**
         * This is called when a characteristic write has completed. Is uses a queue to determine if
         * additional BLE actions are still pending and launches the next one if there are.
         *
         * @param gatt The GATT database object
         * @param characteristic The characteristic that was written.
         * @param status Status of whether the write was successful.
         */
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                                          BluetoothGattCharacteristic characteristic,
                                          int status) {
            // Pop the item that was written from the queue
            BleQueue.remove();
            // See if there are more items in the BLE queues
            handleBleQueue();
        }

        /**
         * This is called when a CCCD write has completed. It uses a queue to determine if
         * additional BLE actions are still pending and launches the next one if there are.
         *
         * @param gatt The GATT database object
         * @param descriptor The CCCD that was written.
         * @param status Status of whether the write was successful.
         */
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                                      int status) {
            // Pop the item that was written from the queue
            BleQueue.remove();
            // See if there are more items in the BLE queues
            handleBleQueue();
        }

        /**
         * This is called when a characteristic with notify set changes.
         * It broadcasts an update to the display activity with the changed data.
         *
         * @param gatt The GATT database object
         * @param characteristic The characteristic that was changed
         */
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            // Get the UUID of the characteristic that changed
            String uuid = characteristic.getUuid().toString();

            // Update the appropriate variable with the new value.
            switch (uuid) {
                case encoderXCharUUID:
                    encoderXValue = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT32,0);
                    break;
                case encoderYCharUUID:
                    encoderYValue = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT32,0);
                    break;
                case encoderZCharUUID:
                    encoderZValue = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_SINT32,0);
                    break;
            }
            // Tell the display activity that new encoder data is available
            broadcastUpdate(ACTION_DATA_AVAILABLE);
        }
    };


    /**
     * Sends a broadcast to the listener in the display activity.
     *
     * @param action The type of action that occurred.
     */
    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }


    /**
     * Initialize a reference to the local Bluetooth adapter.
     *
     * @return Return true if the initialization is successful.
     */
    public boolean initialize() {
        // For API level 18 and above, get a reference to BluetoothAdapter through
        // BluetoothManager.
        if (varBluetoothManager == null) {
            varBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (varBluetoothManager == null) {
                Log.e(TAG, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        varBluetoothAdapter = varBluetoothManager.getAdapter();
        if (varBluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }

        // Initialize encoder variables
        encoderXValue = 0;
        encoderYValue = 0;
        encoderZValue = 0;

        return true;
    }

    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     * @return Return true if the connection is initiated successfully. The connection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public boolean connect(final String address) {
        if (varBluetoothAdapter == null || address == null) {
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        // Previously connected device.  Try to reconnect.
        if (varBluetoothDeviceAddress != null && address.equals(varBluetoothDeviceAddress)
                && varBluetoothGatt != null) {
            Log.i(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            return varBluetoothGatt.connect();
        }

        final BluetoothDevice device = varBluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.w(TAG, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect
        // parameter to false.
        varBluetoothGatt = device.connectGatt(this, false, varGattCallback);
        Log.i(TAG, "Trying to create a new connection.");
        varBluetoothDeviceAddress = address;
        return true;
    }

    /**
     * Disconnects an existing connection or cancel a pending connection. The disconnection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void disconnect() {
        if (varBluetoothAdapter == null || varBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        varBluetoothGatt.disconnect();
    }

    /**
     * After using a given BLE device, the app must call this method to ensure resources are
     * released properly.
     */
    public void close() {
        if (varBluetoothGatt == null) {
            return;
        }
        varBluetoothGatt.close();
        varBluetoothGatt = null;
    }

    public static void sendResetCommand(char resetValue)
    {
        workspace0Characteristic.setValue(resetValue, BluetoothGattCharacteristic.FORMAT_SINT8,0);
        writeCharacteristic(workspace0Characteristic);
    }

    /**
     * Request a write on a given {@code BluetoothGattCharacteristic}.
     *
     * @param characteristic The characteristic to write.
     */
    private static void writeCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (varBluetoothAdapter == null || varBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        BleQueue.add(characteristic);
        if (BleQueue.size() == 1) {
            varBluetoothGatt.writeCharacteristic(characteristic);
            Log.i(TAG, "Writing Characteristic");
        }
    }

    /**
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled        If true, enable notification.  False otherwise.
     */
    private void setCharacteristicNotification(BluetoothGattCharacteristic characteristic,
                                               boolean enabled) {
        if (varBluetoothAdapter == null || varBluetoothGatt == null) {
            Log.i(TAG, "BluetoothAdapter not initialized");
            return;
        }

        /* Enable or disable the callback notification on the phone */
        varBluetoothGatt.setCharacteristicNotification(characteristic, enabled);

        /* Set CCCD value locally and then write to the device to register for notifications */
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                UUID.fromString(CCCD_UUID));
        if (enabled) {
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        } else {
            descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
        }
        // Put the descriptor into the write queue
        BleQueue.add(descriptor);
        // If there is only 1 item in the queue, then write it. If more than one, then the callback
        // will handle it
        if (BleQueue.size() == 1) {
            varBluetoothGatt.writeDescriptor(descriptor);
            Log.i(TAG, "Writing Notification");
        }
    }

    /**
     * Get one of the encoder reading
     *
     * @param encoder to operate on
     * @return encoder value
     */
    public static int getEncoderValue(Encoder encoder) {
        if (encoder == Encoder.ENCODER_X) {
            return encoderXValue;
        } else if(encoder == Encoder.ENCODER_Y){
            return encoderYValue;
        } else {
            return encoderZValue;
        }
    }

    /**
     * This function returns the UUID of the PSoC 6 DRO service
     *
     * @return the PSoC 6 DRO service UUID
     */
    public static UUID getDROServiceUUID() {
        return UUID.fromString(BLEService.DROServiceUUID);
    }
}
