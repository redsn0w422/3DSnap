import UIKit
import MobileCoreServices
import Alamofire

class SecondPictureViewController: UIViewController,
UINavigationControllerDelegate, UIImagePickerControllerDelegate {
    
    /* We will use this variable to determine if the viewDidAppear:
    method of our view controller is already called or not. If not, we will
    display the camera view */
    var beenHereBefore = false
    var controller: UIImagePickerController?
    var count = 0
    
    func imagePickerController(picker: UIImagePickerController,
        didFinishPickingMediaWithInfo info: [String: AnyObject]){
            
            print("Picker returned successfully")
            
            let mediaType:AnyObject? = info[UIImagePickerControllerMediaType]
            
            if let type:AnyObject = mediaType{
                
                if type is String{
                    let stringType = type as! String
                    
                    if stringType == kUTTypeMovie as String{
                        let urlOfVideo = info[UIImagePickerControllerMediaURL] as? NSURL
                        if let url = urlOfVideo{
                            print("Video URL = \(url)")
                        }
                    }
                        
                    else if stringType == kUTTypeImage as String{
                        /* Let's get the metadata. This is only for images. Not videos */
                        let metadata = info[UIImagePickerControllerMediaMetadata]
                            as? NSDictionary
                        if let theMetaData = metadata{
                            let image = info[UIImagePickerControllerOriginalImage]
                                as? UIImage
                            if let theImage = image{
                                print("Image Metadata = \(theMetaData)")
                                print("Image = \(theImage)")
                                var imageData = UIImageJPEGRepresentation(theImage, 0.9)
                                var base64String = imageData!.base64EncodedStringWithOptions(NSDataBase64EncodingOptions(rawValue: 0)) // encode the image
                                print(base64String)
                                
                                
                                let parameters = [
                                    "sendFrom": "niraj",
                                    "sendTo": "yasha",
                                    "image_left": image1string,
                                    "image_right": base64String
                                ]
                                // https://4c576b5e.ngrok.com/send
                                Alamofire.request(.POST, "https://10.186.135.199:3000/send", parameters: parameters)
                                print("Post req sent")
                            }
                        }
                    }
                    
                }
            }
            
            //      picker.dismissViewControllerAnimated(true, completion: nil)
//            picker.dismissViewControllerAnimated(true) { () -> Void in
//                self.performSegueWithIdentifier("secondVC", sender: nil)
//            }
    }
    
    func imagePickerControllerDidCancel(picker: UIImagePickerController) {
        print("Picker was cancelled")
        picker.dismissViewControllerAnimated(true, completion: nil)
    }
    
    func isCameraAvailable() -> Bool{
        return UIImagePickerController.isSourceTypeAvailable(.Camera)
    }
    
    func cameraSupportsMedia(mediaType: String,
        sourceType: UIImagePickerControllerSourceType) -> Bool{
            
            let availableMediaTypes =
            UIImagePickerController.availableMediaTypesForSourceType(sourceType) as
                [String]?
            
            if let types = availableMediaTypes{
                for type in types{
                    if type == mediaType{
                        return true
                    }
                }
            }
            
            return false
    }
    
    func doesCameraSupportTakingPhotos() -> Bool{
        return cameraSupportsMedia(kUTTypeImage as String, sourceType: .Camera)
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        if beenHereBefore{
            /* Only display the picker once as the viewDidAppear: method gets
            called whenever the view of our view controller gets displayed */
            return;
        } else {
            beenHereBefore = true
        }
        
        if isCameraAvailable() && doesCameraSupportTakingPhotos(){
            count++
            print(count)
            controller = UIImagePickerController()
            
            if let theController = controller{
                theController.sourceType = .Camera
                
                theController.mediaTypes = [kUTTypeImage as String]
                
                theController.allowsEditing = true
                theController.delegate = self
                
                presentViewController(theController, animated: true, completion: nil)
            }
            
        } else {
            print("Camera is not available")
        }
    }
    
}

