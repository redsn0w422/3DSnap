package org.boilermake.boilersnap;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;

import org.apache.commons.codec.binary.Base64;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import retrofit.Callback;
import retrofit.RetrofitError;
import retrofit.http.Body;
import retrofit.client.Response;

/**
 * Created by rohan on 17/10/15.
 */
public class Camera2 extends Activity {
    private Camera mCamera;
    private CameraPreview mCameraPreview;
    private Context context;

    private File picture1;
    private File picture2;

    private final String url = "http://google.com";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera2);
        mCamera = Camera1.mCamera;
//        mCamera.startPreview();
        mCameraPreview = new CameraPreview(this, mCamera);
        FrameLayout preview = (FrameLayout) findViewById(R.id.camera_preview);
        preview.addView(mCameraPreview);

        picture1 = (File) getIntent().getExtras().get("picture");

        preview.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mCamera.takePicture(null, null, mPicture);
            }
        });

        context = this;
    }

    /**
     * Helper method to access the camera returns null if it cannot get the
     * camera or does not exist
     *
     * @return
     */
    private Camera getCameraInstance() {
        Camera camera = null;
        try {
            camera = Camera.open();
        } catch (Exception e) {
            // cannot get camera or does not exist
        }
        return camera;
    }

    Camera.PictureCallback mPicture = new Camera.PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            File pictureFile = getOutputMediaFile();
            if (pictureFile == null) {
                return;
            }
            try {
                FileOutputStream fos = new FileOutputStream(pictureFile);
                fos.write(data);
                fos.close();
                mCamera.stopPreview();
                mCamera.release();

                picture2 = pictureFile;

                // retrofit to push the images
                uploadImages(picture1, picture2);

                startActivity(new Intent(context, MainActivity.class));
            } catch (FileNotFoundException e) {

            } catch (IOException e) {
            }
        }
    };

    private static File getOutputMediaFile() {
        File mediaStorageDir = new File(
                Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
                "MyCameraApp");
        if (!mediaStorageDir.exists()) {
            if (!mediaStorageDir.mkdirs()) {
                Log.d("MyCameraApp", "failed to create directory");
                return null;
            }
        }
        // Create a media file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss")
                .format(new Date());
        File mediaFile;
        mediaFile = new File(mediaStorageDir.getPath() + File.separator
                + "IMG_" + timeStamp + ".jpg");
        Log.d("camera2", "stored pic");
        return mediaFile;
    }

    private void uploadImages(File picture1, File picture2) {
        String pictureOneString = "";
        String pictureTwoString = "";

        try {
            pictureOneString = encodeFileToBase64Binary(picture1);
            pictureTwoString = encodeFileToBase64Binary(picture2);
        } catch (Exception e) {
            Log.e("rip", "rip making the base64 image");
        }


        boolean result = false;
        HttpClient hc = new DefaultHttpClient();
        String message;

        HttpPost p = new HttpPost(url);
        JSONObject object = new JSONObject();
        try {

            object.put("fromUsername", "lol");
            object.put("toUsername", "boilermake");
            object.put("pictureOne", pictureOneString);
            object.put("pictureTwo", pictureTwoString);

        } catch (Exception ex) {

        }

        try {
            message = object.toString();


            p.setEntity(new StringEntity(message, "UTF8"));
            p.setHeader("Content-type", "application/json");
            HttpResponse resp = hc.execute(p);
            if (resp != null) {
                if (resp.getStatusLine().getStatusCode() == 204)
                    result = true;
            }

            Log.d("Status line", "" + resp.getStatusLine().getStatusCode());
        } catch (Exception e) {
            e.printStackTrace();

        }
    }

    private String encodeFileToBase64Binary(File file) throws IOException {

        byte[] bytes = loadFile(file);
        byte[] encoded = Base64.encodeBase64(bytes);
        String encodedString = new String(encoded);

        return encodedString;
    }

    private static byte[] loadFile(File file) throws IOException {
        InputStream is = new FileInputStream(file);

        long length = file.length();
        if (length > Integer.MAX_VALUE) {
            // File is too large
        }
        byte[] bytes = new byte[(int)length];

        int offset = 0;
        int numRead = 0;
        while (offset < bytes.length
                && (numRead=is.read(bytes, offset, bytes.length-offset)) >= 0) {
            offset += numRead;
        }

        if (offset < bytes.length) {
            throw new IOException("Could not completely read file "+file.getName());
        }

        is.close();
        return bytes;
    }
}
