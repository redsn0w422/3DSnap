import UIKit

class ImageViewController: UIViewController {

    @IBOutlet weak var theImageView: UIImageView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        let fileUrl = NSURL(string: urlString)
        theImageView.setImageWithUrl(fileUrl!)
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
}
