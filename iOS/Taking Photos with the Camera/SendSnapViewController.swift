import UIKit
import Alamofire

class SendSnapViewController: UIViewController {
    @IBOutlet weak var sendFromTextField: UITextField!
    @IBOutlet weak var sendToTextField: UITextField!
    @IBAction func sendSnapButtonClicked(sender: AnyObject) {
        
        let parameters = [
            "sendFrom": sendFromTextField.text!,
            "sendTo": sendToTextField.text!,
            "image_left": image1string,
            "image_right": image2string
        ]
        
        Alamofire.request(.POST, "http://159.203.98.104:3000/send", parameters: parameters)
        print("Post req sent")

    }
    override func viewDidLoad() {
        super.viewDidLoad()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
}
