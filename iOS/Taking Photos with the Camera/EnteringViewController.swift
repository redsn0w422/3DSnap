import UIKit
import Alamofire

class EnteringViewController: UIViewController {
    @IBOutlet weak var usernameField: UITextField!

    @IBAction func viewSnapButtonClicked(sender: AnyObject) {
        Alamofire.request(.GET, "https://4c576b5e.ngrok.com/get", parameters: ["username": usernameField.text!, "client": "mobile"])
            .responseJSON { response in
                print(response.request)  // original URL request
                print(response.response) // URL response
                print(response.data)     // server data
                print(response.result)   // result of response serialization
                
                if let JSON = response.result.value {
                    print("JSON: \(JSON)")
                }
        }
        // if returns image, segue
        // if not, alert modal
    }
    
    @IBAction func takeSnapButtonClicked(sender: AnyObject) {
        self.performSegueWithIdentifier("takeSnap", sender: nil)
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
