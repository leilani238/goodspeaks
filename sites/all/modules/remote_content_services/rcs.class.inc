<?php

class rcsException extends Exception {}

/**
 * Simple factory to register implementing classes and
 * create entities and fields to export.
 */
class rcsFactory {
  private static $classes;

  /**
   * Register implementing classes via drupal hooks.
   * Modules wishing to register their own classes should
   * implement hook_rcs_classes(); These should return an
   * array with two elements: 'entities' and 'fields' e.g.
   *
   *  return array(
   *    'files' => array(
   *      drupal_get_path('module', 'yourmodule') . '/yourmodule.rcs.inc',
   *    ),
   *    'entities' => array(
   *      'node' => 'rcsEntityNode',
   *    ),
   *    'fields' => array(
   *      'textfield' => 'rcsFieldStandard',
   *      '*' => 'rcsFieldStandard',
   *    ),
   *  );
   */
  public static function registry() {
    if (!empty(self::$classes)) {
      return self::$classes;
    }

    $classes = array();
    $possible_classes = module_invoke_all('rcs_classes');

    if (!empty($possible_classes['files'])) {
      foreach ($possible_classes['files'] as $path) {
        if (file_exists($path)) {
          include_once $path;
        }
      }
    }
    foreach (array('entities', 'fields') as $class_type) {
      if (!empty($possible_classes[$class_type])) {
        foreach ($possible_classes[$class_type] as $type => $suggested) {
          $classes[$class_type][$type] = is_array($suggested) ? array_pop($suggested) : $suggested;
        }
      }
    }

    drupal_alter('rcs_classes', $classes);

    self::$classes = $classes;
  }

  /**
   * Factory to create entities based on the registry.
   */
  public static function createEntity($type, $attrs) {
    if (isset(self::$classes['entities'][$type])) {
      $class = self::$classes['entities'][$type];

      if (class_exists($class)) {
        return new $class($attrs);
      }
    }

    throw new rcsException('RCS entity class "' . $class . '" not found.');
  }

  /**
   * Factory to create fields based on the registry.
   */
  public static function createField($type, $attrs) {
    if (isset(self::$classes['fields'][$type])) {
      $class = self::$classes['fields'][$type];
    }
    elseif (isset(self::$classes['fields']['*'])) {
      $class = self::$classes['fields']['*'];
    }

    if (class_exists($class)) {
      return new $class($attrs);
    }

    throw new rcsException('RCS field class "' . $class . '" not found.');
  }

}

/**
 * Defines a entity that RCS can expose.
 */
abstract class rcsEntity {

  protected $fields;
  protected $attrs;

  /**
   * Constructor.
   */
  public function __construct($attrs = array()) {
    $this->attrs = $attrs;
    $this->createFields();
  }

  /**
   * Set a field.
   */
  public function addField($name, $type, $attrs) {
    $this->fields[$name] = rcsFactory::createField($type, $attrs);
  }  

  /**
   * Get a field.
   */
  public function getField($name = NULL) {
    if (!$name) {
      return $this->fields;
    }
    return $this->fields[$name];
  }  

  /**
   * Get all fields.
   */
  public function getFields($name = NULL) {
    $fields = array();
    foreach ($this->fields as $name => $field) {
      $fields[$name] = $field->getField();
    }
    return $fields;
  }
 
  protected abstract function createFields();

  /**
   * Get create permission.
   *
   * @param string $type.
   */
  public abstract function canCreate($type);

  /**
   * Get read permission.
   *
   * @param string $entity.
   */
  public abstract function canRead($entity);

  /**
   * Get update permission.
   *
   * @param string $entity.
   */
  public abstract function canUpdate($entity);

  /**
   * Get delete permission.
   *
   * @param string $entity.
   */
  public abstract function canDelete($entity);
}

/**
 * Defines a node form.
 */
class rcsEntityNode extends rcsEntity {
  protected function createFields() {
    $fields = field_info_fields();
    $instances = field_info_instances();

    $this->addField(
      'title',
      'textfield',
      array(
        'field' => array('type' => 'textfield', 'cardinality' => 1),
        'instance' => array('label' => t('Title'), 'default' => '', 'required' => TRUE),
      )
    );

    if (user_access('administer nodes')) {
      $this->addField(
        'author',
        'textfield',
        array(
          'field' => array('type' => 'textfield', 'cardinality' => 1),
          'instance' => array('label' => t('Author'), 'default' => '', 'required' => TRUE),
        )
      );
    }

    foreach ($instances['node'][$this->attrs['type']] as $instance) {
      $field = $fields[$instance['field_name']];
      $this->addField(
        $instance['field_name'],
        $field['type'],
        array(
          'field' => $field,
          'instance' => $instance,
        )
      );
    }
  }

  public function canCreate($type) {
    return node_access('create', $type);
  }

  public function canRead($entity) {
    return node_access('create', $view);
  }

  public function canUpdate($entity) {
    return node_access('create', $update);
  }

  public function canDelete($entity) {
    return node_access('create', $delete);
  }
}

/**
 * Defines a field that RCS can expose.
 */
abstract class rcsField {
  protected $attrs;
  protected $field;

  /**
   * Defines a field that RCS can expose.
   *
   * @param Unknown $attrs
   */
  public function __construct($attrs) {
    $this->attrs = $attrs;
    $this->createField(); 
  }

  /**
   * @return array
   *  A list of attributs including: 
   *    - type
   *    - label
   *    - cardinality
   *    - default
   *    - required
   */
  public function getField() {
    return $this->field;
  }

  public abstract function createField();

  public abstract function autoComplete();
}

/**
 * Most fields can be handled in a standard way.
 */
class rcsFieldStandard extends rcsField {
  public function createField() {
    $this->field['type'] = $this->attrs['field']['type'];
    $this->field['label'] = t($this->attrs['instance']['label']);
    $this->field['cardinality'] = $this->attrs['field']['cardinality'];
    $this->field['default'] = $this->attrs['instance']['default_value'][0]['value'];
    $this->field['required'] = $this->attrs['instance'] ? TRUE : FALSE;
  }

  public function autoComplete() {
    return $this->attrs['field']['settings']['allowed_values'];
  }
}

class rcsFieldTextfield extends rcsFieldStandard {}
class rcsFieldTextWithSummary extends rcsFieldStandard {}
