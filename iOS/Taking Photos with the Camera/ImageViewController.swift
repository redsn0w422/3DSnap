import UIKit
import Alamofire

class ImageViewController: UIViewController {

    @IBOutlet weak var theImageView: UIImageView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        let fileUrl = NSURL(string: urlString)
        Alamofire.request(.GET, urlString)
            .response { (request, response, data, error) in
                self.theImageView.image = UIImage(data: data!, scale: 1)
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
}
