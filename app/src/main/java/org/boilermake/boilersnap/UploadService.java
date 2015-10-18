package org.boilermake.boilersnap;

import retrofit.Callback;
import retrofit.http.Body;
import retrofit.http.POST;

public interface UploadService {
    @POST("/tasks")
    void uploadImages(@Body Payload task, Callback<String> cb);
}
