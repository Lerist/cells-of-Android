package com.cells.cellswitch.secure.view;

import android.app.Activity;
import android.app.Dialog;
import android.util.Log;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.Looper;
import android.os.RemoteException;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.widget.Toast;
import android.widget.Button;
import java.io.IOException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import com.cells.cellswitch.secure.R;
import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.CellsPrivateServiceManager;

import android.bluetooth.BluetoothAdapter;
import android.nfc.NfcAdapter;

public class SwitchActivity extends Activity {
	private static final String TAG = "SwitchActivity";
	private static final int MSG_UP_CODE = 0x01;
	private static final int MSG_START_VM_CODE = 0x02;
	private static final int MSG_VM_BACK_CODE = 0x03;
	private static final int MSG_START_VM_TOAST_CODE = 0x04;
	private static final int MSG_P2P_CODE = 0x05;
	private static final int REQUEST_PERMISSION_CODE = 0x088;
	private Dialog mWeiboDialog;
	private WakeLock mWakeLock = null;
	private CellsPrivateServiceManager mCellsService = null;
	private Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			switch (msg.what) {
				case MSG_START_VM_CODE:
				{
					start();
					break;
				}
				case MSG_START_VM_TOAST_CODE:
				{
					Toast.makeText(SwitchActivity.this, "容器启动失败.", Toast.LENGTH_SHORT).show();
				}
			}
		}
	};

	private void acquireWakeLock() {
		if (mWakeLock == null) {
			PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);     
			mWakeLock = pm.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP |
									 PowerManager.FULL_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE, TAG);
			mWakeLock.acquire();
		}
	}

	private void releaseWakeLock() {
		if (mWakeLock != null && mWakeLock.isHeld()) {
			mWakeLock.release();
			mWakeLock = null;
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		acquireWakeLock();

		mCellsService = new CellsPrivateServiceManager(SwitchActivity.this);

		int isPermission = ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
		if (isPermission == PackageManager.PERMISSION_DENIED) {
			ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_PERMISSION_CODE);
		}

		if(SystemProperties.get("ro.boot.vm").equals("1")){
			Button btn_up_vm = (Button) findViewById(R.id.btn_up_vm);
			btn_up_vm.setVisibility(View.GONE);
			Button btn_down_vm = (Button) findViewById(R.id.btn_down_vm);
			btn_down_vm.setVisibility(View.GONE);
			Button btn_start_vm = (Button) findViewById(R.id.btn_start_vm);
			btn_start_vm.setVisibility(View.GONE);
			Button btn_stop_vm = (Button) findViewById(R.id.btn_stop_vm);
			btn_stop_vm.setVisibility(View.GONE);
			Button btn_connect_vm = (Button) findViewById(R.id.btn_connect_vm);
			btn_connect_vm.setVisibility(View.GONE);
			Button btn_group_vm = (Button) findViewById(R.id.btn_group_vm);
			btn_group_vm.setVisibility(View.GONE);
		}else{
			Button btn_up_vm = (Button) findViewById(R.id.btn_up_vm);
			btn_up_vm.setVisibility(View.GONE);
			Button btn_back_vm = (Button) findViewById(R.id.btn_back_vm);
			btn_back_vm.setVisibility(View.GONE);
			Button btn_down_vm = (Button) findViewById(R.id.btn_down_vm);
			btn_down_vm.setVisibility(View.GONE);
			Button btn_connect_vm = (Button) findViewById(R.id.btn_connect_vm);
			btn_connect_vm.setVisibility(View.GONE);
			Button btn_group_vm = (Button) findViewById(R.id.btn_group_vm);
			btn_group_vm.setVisibility(View.GONE);
		}
	}

	private void enableordisable(Boolean  e){
		if(SystemProperties.get("ro.boot.vm").equals("1")){
			Button btn_back_vm = (Button) findViewById(R.id.btn_back_vm);
			btn_back_vm.setEnabled(e);
		}else{
			Button btn_up_vm = (Button) findViewById(R.id.btn_up_vm);
			btn_up_vm.setEnabled(e);
			Button btn_start_vm = (Button) findViewById(R.id.btn_start_vm);
			btn_start_vm.setEnabled(e);
			Button btn_stop_vm = (Button) findViewById(R.id.btn_stop_vm);
			btn_stop_vm.setEnabled(e);
			Button btn_connect_vm = (Button) findViewById(R.id.btn_connect_vm);
			btn_connect_vm.setEnabled(e);
			Button btn_group_vm = (Button) findViewById(R.id.btn_group_vm);
			btn_group_vm.setEnabled(e);
		}
	}

	private void exit(){
		android.os.Process.killProcess(android.os.Process.myPid());
	}

	@Override
	public void onDestroy() {
		releaseWakeLock();
		super.onDestroy();
	}

	public void btnBack(View v){
		ExitAnimation myYAnimation = new ExitAnimation();
		Animation.AnimationListener  animationListener = new Animation.AnimationListener() {
			@Override
			public void onAnimationEnd(Animation animation) {
					try{
						mCellsService.switchCellsVM("cell1");
					}catch(RemoteException e){
						e.printStackTrace();
					}
					enableordisable(true);
					exit();
			}
			@Override
			public void onAnimationStart(Animation animation) {
				enableordisable(false);
			}
			@Override
			public void onAnimationRepeat(Animation animation) {

			}
		};

		BluetoothAdapter blueadapter = BluetoothAdapter.getDefaultAdapter();
		blueadapter.disable();

		NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(SwitchActivity.this);
		nfcAdapter.disable();

		myYAnimation.setAnimationListener(animationListener); 
		getWindow().getDecorView().findViewById(R.id.main_layout).startAnimation(myYAnimation);
	}

	public void start(){
		if(mWeiboDialog != null){
			WeiboDialogUtils.closeDialog(mWeiboDialog);
			mWeiboDialog = null;
		}

		ExitAnimation myYAnimation = new ExitAnimation();
		Animation.AnimationListener  animationListener = new Animation.AnimationListener() {
			@Override
			public void onAnimationEnd(Animation animation) {
					if(SystemProperties.get("persist.sys.vm.init").equals("1")){
						try{
							mCellsService.switchCellsVM("cell1");
						}catch(RemoteException e){
							e.printStackTrace();
						}
						enableordisable(true);
						exit();
					}else{
						mHandler.sendEmptyMessage(MSG_START_VM_TOAST_CODE);
						enableordisable(true);
					}
			}
			@Override
			public void onAnimationStart(Animation animation) {
				enableordisable(false);
			}
			@Override
			public void onAnimationRepeat(Animation animation) {

			}
		};

		BluetoothAdapter blueadapter = BluetoothAdapter.getDefaultAdapter();
		blueadapter.disable();

		NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(SwitchActivity.this);
		nfcAdapter.disable();

		myYAnimation.setAnimationListener(animationListener); 
		getWindow().getDecorView().findViewById(R.id.main_layout).startAnimation(myYAnimation);
	}

	public void btnStart(View v){
		mWeiboDialog = WeiboDialogUtils.createLoadingDialog(SwitchActivity.this, SwitchActivity.this.getString(R.string.starting_string));
		new Thread(new Runnable() {
					@Override
					public void run() {
						if(SystemProperties.get("persist.sys.vm.init").equals("1")){
							mHandler.sendEmptyMessage(MSG_START_VM_CODE);
						}else{
							long beginTime=System.currentTimeMillis();
							try{
								mCellsService.startCellsVM("cell1");
							}catch(RemoteException e){
								e.printStackTrace();
							}
							long ms = System.currentTimeMillis() - beginTime;
							Log.e(TAG, "启动镜像消耗 - " + ms + "(ms).");
							try {
								Thread.sleep(6000);
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
							mHandler.sendEmptyMessage(MSG_START_VM_CODE);
						}
					}
		}).start();
	}

	public void btnStop(View v){
		try{
			mCellsService.stopCellsVM("cell1");
		}catch(RemoteException e){
			e.printStackTrace();
		}
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		Toast.makeText(SwitchActivity.this,"关闭成功",Toast.LENGTH_SHORT).show();
	}

	public void btnDown(View v){

	}

	public void btnUp(View v){

	}
}